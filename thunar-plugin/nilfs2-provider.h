#ifndef __NILFS2_PROVIDER_H__
#define __NILFS2_PROVIDER_H__

#include <thunarx/thunarx.h>

G_BEGIN_DECLS;

typedef struct _NILFS2ProviderClass NILFS2ProviderClass;
typedef struct _NILFS2Provider      NILFS2Provider;

#define TYPE_NILFS2_PROVIDER            (nilfs2_provider_get_type ())
#define NILFS2_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_NILFS2_PROVIDER, NILFS2Provider))
#define NILFS2_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  TYPE_NILFS2_PROVIDER, NILFS2ProviderClass))
#define IS_NILFS2_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_NILFS2_PROVIDER))
#define IS_NILFS2_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  TYPE_NILFS2_PROVIDER))
#define NILFS2_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  TYPE_NILFS2_PROVIDER, NILFS2ProviderClass))

GType    nilfs2_provider_get_type        (void) G_GNUC_CONST G_GNUC_INTERNAL;
void     nilfs2_provider_register_type   (ThunarxProviderPlugin *plugin) G_GNUC_INTERNAL;

//gboolean nilfs2_get_audio_file_supported (ThunarxFileInfo *info) G_GNUC_INTERNAL;

G_END_DECLS;

#endif
