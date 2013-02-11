#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <nilfs2-provider.h>
#include <nilfs2-page.h>

static void   nilfs2_provider_nilfs2_page_provider_init (ThunarxPropertyPageProviderIface *iface);
static GList *nilfs2_provider_get_pages                 (ThunarxPropertyPageProvider      *nilfs2_provider,
                                                         GList                            *files);
static gboolean nilfs2_provider_supported               (ThunarxFileInfo                  *info);

struct _NILFS2ProviderClass
{
    GObjectClass __parent__;
};

struct _NILFS2Provider
{
    GObject __parent__;
};

THUNARX_DEFINE_TYPE_WITH_CODE (NILFS2Provider,
                               nilfs2_provider,
                               G_TYPE_OBJECT,
                               THUNARX_IMPLEMENT_INTERFACE (THUNARX_TYPE_PROPERTY_PAGE_PROVIDER,
                                                            nilfs2_provider_nilfs2_page_provider_init));

static void nilfs2_provider_class_init (NILFS2ProviderClass *klass)
{
}

static void nilfs2_provider_init (NILFS2Provider *sbr_provider)
{
}

static void nilfs2_provider_nilfs2_page_provider_init (ThunarxPropertyPageProviderIface *iface)
{
    iface->get_pages = nilfs2_provider_get_pages;
}

static GList* nilfs2_provider_get_pages (ThunarxPropertyPageProvider *page_provider, GList *files)
{
    GList           *pages = NULL;
    GList           *file;
    ThunarxFileInfo *info;

    if (g_list_length (files) != 1) 
        return NULL;
  
    file = g_list_first (files);

    if (G_UNLIKELY (file == NULL))
        return NULL;

    info = THUNARX_FILE_INFO (file->data);

    if (G_LIKELY (nilfs2_provider_supported(info))) {
        NILFS2Page *page;
        g_message ("nilfs2_provider_get_pages");
        page = nilfs2_page_new ();
        nilfs2_page_set_file (page, info);
        pages = g_list_prepend (pages, page);
    }

    return pages;
}

static gboolean nilfs2_provider_supported(ThunarxFileInfo *info)
{
    gchar   *uri;
    gchar   *filename;
    gboolean supported = FALSE;
    struct stat s, s_dev;
    int ret;

    uri = thunarx_file_info_get_uri (info);
    filename = g_filename_from_uri (uri, NULL, NULL);
    ret = stat(filename, &s);
    g_free (uri);
    g_free (filename);

    if (ret == 0) {

        char buf[1024];
        char dev[512];
        char fsn[8];
        FILE *fp = fopen("/proc/mounts", "r");
        while(fgets(buf, 1024, fp)) {
            sscanf(buf, "%511s %*s %7s", dev, fsn);
            if (strncmp("nilfs2", fsn, 6) != 0)
                continue;
            if (stat(dev, &s_dev) != 0)
                continue;
            if (s.st_dev == s_dev.st_rdev) {
                supported = TRUE;
                break;
            }
        }
        fclose(fp);

    }

    return supported;
}
