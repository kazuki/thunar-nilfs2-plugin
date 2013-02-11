#ifndef __NILFS2_PAGE_H__
#define __NILFS2_PAGE_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _NILFS2PageClass NILFS2PageClass;
typedef struct _NILFS2Page      NILFS2Page;

#define TYPE_NILFS2_PAGE            (nilfs2_page_get_type ())
#define NILFS2_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_NILFS2_PAGE, NILFS2Page))
#define NILFS2_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_NILFS2_PAGE, NILFS2PageClass))
#define IS_NILFS2_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_NILFS2_PAGE))
#define IS_NILFS2_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_NILFS2_PAGE))
#define NILFS2_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_NILFS2_PAGE, NILFS2PageClass))

GType            nilfs2_page_get_type             (void) G_GNUC_CONST G_GNUC_INTERNAL;
void             nilfs2_page_register_type        (ThunarxProviderPlugin *plugin) G_GNUC_INTERNAL;

NILFS2Page *nilfs2_page_new                  (void) G_GNUC_CONST G_GNUC_INTERNAL G_GNUC_MALLOC;
#if 0
NILFS2Page *nilfs2_page_new_with_save_button (void) G_GNUC_CONST G_GNUC_INTERNAL G_GNUC_MALLOC;
GtkWidget  *nilfs2_page_dialog_new           (GtkWindow       *window, 
                                              ThunarxFileInfo *file) G_GNUC_CONST G_GNUC_INTERNAL G_GNUC_MALLOC;
#endif

ThunarxFileInfo *nilfs2_page_get_file             (NILFS2Page      *page) G_GNUC_INTERNAL;
void             nilfs2_page_set_file             (NILFS2Page      *page,
                                                   ThunarxFileInfo *file) G_GNUC_INTERNAL;
#if 0
TagLib_File     *nilfs2_page_get_taglib_file      (NILFS2Page      *page) G_GNUC_INTERNAL;
void             nilfs2_page_set_taglib_file      (NILFS2Page      *page,
                                                   TagLib_File     *file) G_GNUC_INTERNAL;
gboolean         nilfs2_page_get_show_save_button (NILFS2Page      *page) G_GNUC_INTERNAL;
void             nilfs2_page_set_show_save_button (NILFS2Page      *page,
                                                   gboolean         show) G_GNUC_INTERNAL;
#endif

G_END_DECLS;

#endif
