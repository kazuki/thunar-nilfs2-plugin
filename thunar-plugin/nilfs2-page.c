#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <pthread.h>
#include <exo/exo.h>
#include <nilfs2-page.h>
#include <nilfs.h>

enum {
    PROP_0,
    PROP_FILE,
};

static void nilfs2_page_finalize     (GObject         *object);
static void nilfs2_page_get_property (GObject         *object,
                                      guint            prop_id,
                                      GValue          *value,
                                      GParamSpec      *pspec);
static void nilfs2_page_set_property (GObject         *object,
                                      guint            prop_id,
                                      const GValue    *value,
                                      GParamSpec      *pspec);
static void nilfs2_page_file_changed (ThunarxFileInfo *file,
                                      NILFS2Page      *page);
static void* nilfs2_find_cp_thread   (void*);
static void nilfs2_find_checkpoints  (NILFS2Page      *page,
                                      __u64            inode,
                                      volatile int*    shutdown);
static gboolean nilfs2_update_list   (gpointer         data);

struct _NILFS2PageClass
{
    ThunarxPropertyPageClass __parent__;
};

struct _NILFS2Page
{
    ThunarxPropertyPage __parent__;

    /* Widgets */
    GtkWidget       *listview;

    /* Properties */
    ThunarxFileInfo *file;

    /* nilfs2 checkpoint search thread */
    pthread_mutex_t lock;
    pthread_t       thread;
    int             shutdown;
    __u64           ino;
    guint           dispatched;
    GList          *checkpoints;
};
struct _NILFS2CPInfo
{
    __u64 time;
    __u64 cp;
    __u64 segno;
};
typedef struct _NILFS2CPInfo NILFS2CPInfo;

THUNARX_DEFINE_TYPE (NILFS2Page, nilfs2_page, THUNARX_TYPE_PROPERTY_PAGE);

static void nilfs2_page_class_init (NILFS2PageClass *klass)
{
    GObjectClass             *gobject_class;

    gobject_class = G_OBJECT_CLASS (klass);
    gobject_class->finalize = nilfs2_page_finalize;
    gobject_class->get_property = nilfs2_page_get_property;
    gobject_class->set_property = nilfs2_page_set_property;

    g_object_class_install_property (gobject_class,
                                     PROP_FILE,
                                     g_param_spec_object ("file",
                                                          "file",
                                                          "file",
                                                          THUNARX_TYPE_FILE_INFO,
                                                          G_PARAM_READWRITE));
}

static void nilfs2_page_init (NILFS2Page *page)
{
}

static void nilfs2_page_finalize (GObject *object)
{
    NILFS2Page *page = NILFS2_PAGE (object);
    pthread_mutex_lock(&page->lock);
    page->shutdown = 1;
    pthread_mutex_unlock(&page->lock);

    nilfs2_page_set_file (page, NULL);

    pthread_join(page->thread, NULL);
    pthread_mutex_destroy(&page->lock);

    (*G_OBJECT_CLASS (nilfs2_page_parent_class)->finalize) (object);
    g_message ("nilfs2_page_finalize");
}

NILFS2Page* nilfs2_page_new (void)
{
    GtkWidget *vbox;
    GtkWidget *swin;
    GtkListStore *store;
    GtkTreeIter iter;
    NILFS2Page *page = g_object_new (TYPE_NILFS2_PAGE, NULL);
    thunarx_property_page_set_label (THUNARX_PROPERTY_PAGE (page), _("NILFS2"));

    pthread_mutex_init(&page->lock, NULL);
    page->shutdown = 1;
    page->dispatched = 0;
    page->checkpoints = NULL;

    /* Main container */
    vbox = gtk_vbox_new (FALSE, 8);
    gtk_container_add (GTK_CONTAINER (page), vbox);
    gtk_widget_show (vbox);

    swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add (GTK_CONTAINER (vbox), swin);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (swin);

    page->listview = gtk_tree_view_new();
    gtk_container_add (GTK_CONTAINER (swin), page->listview);
    gtk_widget_show (page->listview);

    store = gtk_list_store_new(3, 
                               G_TYPE_STRING,  //  date
                               G_TYPE_UINT64,  //  checkpoint-no
                               G_TYPE_UINT64   //  segment-no
                               );
    gtk_tree_view_set_model(GTK_TREE_VIEW(page->listview), GTK_TREE_MODEL(store));
    gtk_tree_view_append_column(GTK_TREE_VIEW(page->listview), gtk_tree_view_column_new_with_attributes("date", gtk_cell_renderer_text_new(), "text", 0, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(page->listview), gtk_tree_view_column_new_with_attributes("cp no.", gtk_cell_renderer_text_new(), "text", 1, NULL));
    gtk_tree_view_append_column(GTK_TREE_VIEW(page->listview), gtk_tree_view_column_new_with_attributes("seg no.", gtk_cell_renderer_text_new(), "text", 2, NULL));
    g_object_unref(store);


    return page;
}

static void nilfs2_page_get_property (GObject            *object,
                                      guint               prop_id,
                                      GValue             *value,
                                      GParamSpec         *pspec)
{
    NILFS2Page *page = NILFS2_PAGE (object);
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

static void nilfs2_page_set_property (GObject            *object,
                                      guint               prop_id,
                                      const GValue       *value,
                                      GParamSpec         *pspec)
{
    NILFS2Page *page = NILFS2_PAGE (object);
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
}

ThunarxFileInfo* nilfs2_page_get_file (NILFS2Page *page)
{
    g_return_val_if_fail (IS_NILFS2_PAGE (page), NULL);
    return page->file;
}

void nilfs2_page_set_file (NILFS2Page *page, ThunarxFileInfo *file)
{
    g_return_if_fail (IS_NILFS2_PAGE (page));
    g_return_if_fail (file == NULL || THUNARX_IS_FILE_INFO (file));

    /* Check if we already use this file */
    if (G_UNLIKELY (page->file == file))
        return;

    /* Disconnect from the previous file (if any) */
    if (G_LIKELY (page->file != NULL)) {
        g_signal_handlers_disconnect_by_func (G_OBJECT (page->file), nilfs2_page_file_changed, page);
        g_object_unref (G_OBJECT (page->file));
    }

    /* Assign the value */
    page->file = file;

    /* Connect to the new file (if any) */
    if (G_LIKELY (file != NULL)) {
        /* Take a reference on the info file */
        g_object_ref (G_OBJECT (page->file));

        nilfs2_page_file_changed (file, page);

        g_signal_connect (G_OBJECT (file), "changed", G_CALLBACK (nilfs2_page_file_changed), page);
    }
}

static void nilfs2_page_file_changed (ThunarxFileInfo *file, NILFS2Page *page)
{
    int thread_join_flag = 0, ret;
    gchar *uri, *filename;
    struct stat s;

    g_return_if_fail (THUNARX_IS_FILE_INFO (file));
    g_return_if_fail (IS_NILFS2_PAGE (page));
    g_return_if_fail (page->file == file);

    // shutdown running thread
    pthread_mutex_lock(&page->lock);
    if (!page->shutdown) {
        page->shutdown = 1;
        thread_join_flag = 1;
    }
    pthread_mutex_unlock(&page->lock);
    if (thread_join_flag)
        pthread_join(page->thread, NULL);
    page->shutdown = 0;

    // reset ui
    // TODO::

    // find i-node no
    uri = thunarx_file_info_get_uri (page->file);
    filename = g_filename_from_uri (uri, NULL, NULL);
    ret = stat(filename, &s);
    g_free (uri);
    g_free (filename);
    if (ret == 0) {
        page->ino = s.st_ino;
        pthread_create(&page->thread, NULL, nilfs2_find_cp_thread, page);
    }
}

static void* nilfs2_find_cp_thread (void *data)
{
    NILFS2Page *page = NILFS2_PAGE (data);
    nilfs2_find_checkpoints(page, page->ino, &page->shutdown);
    return NULL;
}

static inline int nilfs_suinfo_reclaimable(const struct nilfs_suinfo *si)
{
    return nilfs_suinfo_dirty(si) &&
        /*!nilfs_suinfo_active(si) &&*/ !nilfs_suinfo_error(si);
}
static void nilfs2_find_checkpoints(NILFS2Page *page, __u64 ino, volatile int *shutdown)
{
    __u64 segnum = 0, cpno = 0, cptime = 0;
    ssize_t nsi;
    struct nilfs_sustat sustat;
    struct nilfs_suinfo si;
    void *segment;
    struct nilfs_psegment psegment;
    struct nilfs_file file;
    struct nilfs* nilfs = nilfs_open(NULL, NULL, NILFS_OPEN_RDONLY | NILFS_OPEN_RAW);
    if (nilfs == NULL) {
        g_message("cannot open NILFS");
        return;
    }

    if (nilfs_get_sustat(nilfs, &sustat) < 0) {
        g_message("failed: nilfs_get_sustat");
        goto failed;
    }
    segnum = sustat.ss_nsegs - 1;
    for (; segnum != 0xffffffffffffffff && !*shutdown; --segnum) {
        nsi = nilfs_get_suinfo(nilfs, segnum, &si, 1);
        if (nsi < 0 || *shutdown) {
            g_message("failed: nilfs_get_suinfo");
            break;
        }
        if (!nilfs_suinfo_reclaimable(&si)) {
            continue;
        }
        if (nilfs_get_segment(nilfs, segnum, &segment) < 0) {
            g_message("failed: nilfs_get_segment");
            break;
        }
        nilfs_psegment_for_each(&psegment, segnum, segment, si.sui_nblocks, nilfs) {
            if (*shutdown) break;
            if (psegment.p_segsum) {
                cptime = le64_to_cpu(psegment.p_segsum->ss_create);
                cpno = le64_to_cpu(psegment.p_segsum->ss_cno);
            }
            nilfs_file_for_each(&file, &psegment) {
                if (*shutdown) break;
                if (ino == le64_to_cpu(file.f_finfo->fi_ino)) {
                    NILFS2CPInfo *info = (NILFS2CPInfo*)malloc(sizeof(NILFS2CPInfo));
                    info->time = cptime;
                    info->cp = cpno;
                    info->segno = segnum;
                    pthread_mutex_lock(&page->lock);
                    page->checkpoints = g_list_append(page->checkpoints, info);
                    page->dispatched = g_idle_add(nilfs2_update_list, page);
                    pthread_mutex_unlock(&page->lock);
                    break;
                }
            }
        }
        nilfs_put_segment(nilfs, segment);
    }
failed:
    nilfs_close(nilfs);
}

static void free_helper(gpointer data, gpointer user_data) { free(data); }
static void nilfs2_update_list_insert(gpointer data, gpointer user_data)
{
    GtkListStore *store = GTK_LIST_STORE(user_data);
    NILFS2CPInfo *info = (NILFS2CPInfo*)data;
    GtkTreeIter iter;
    time_t tmp_time  = (time_t)info->time;
    char buf[32];
    struct tm stm;
    localtime_r(&tmp_time, &stm);
    strftime(buf, 32, "%F %T", &stm);
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, buf, 1, info->cp, 2, info->segno, -1);
}

static gboolean nilfs2_update_list(gpointer data)
{
    NILFS2Page *page = NILFS2_PAGE (data);
    GList *list;
    GtkListStore *store;
    pthread_mutex_lock(&page->lock);
    list = page->checkpoints;
    page->checkpoints = NULL;
    page->dispatched = 0;
    pthread_mutex_unlock(&page->lock);

    store = GTK_LIST_STORE(gtk_tree_view_get_model (GTK_TREE_VIEW(page->listview)));
    g_list_foreach(list, (GFunc)nilfs2_update_list_insert, store);
    g_list_foreach(list, (GFunc)free_helper, NULL);
    g_list_free(list); 

    return FALSE;
}
