#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <exo/exo.h>
#include <nilfs2-provider.h>
#include <nilfs2-page.h>

G_MODULE_EXPORT void thunar_extension_initialize (ThunarxProviderPlugin *plugin);
G_MODULE_EXPORT void thunar_extension_shutdown   (void);
G_MODULE_EXPORT void thunar_extension_list_types (const GType **types,
                                                  gint         *n_types);

static GType type_list[1];

G_MODULE_EXPORT void
thunar_extension_initialize (ThunarxProviderPlugin *plugin)
{
    const gchar *mismatch;

    /* verify that the thunarx versions are compatible */
    mismatch = thunarx_check_version (THUNARX_MAJOR_VERSION, THUNARX_MINOR_VERSION, THUNARX_MICRO_VERSION);
    if (G_UNLIKELY (mismatch != NULL)) {
        g_warning ("Version mismatch: %s", mismatch);
        return;
    }

    /* setup i18n support */
    bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
#endif

    //#ifdef G_ENABLE_DEBUG
    g_message ("Initializing thunar-nilfs2-plugin extension");
    //#endif

    /* Register the types provided by this plugin */
    nilfs2_provider_register_type (plugin);
    nilfs2_page_register_type (plugin);

    /* Setup the plugin provider type list */
    type_list[0] = TYPE_NILFS2_PROVIDER;
}



G_MODULE_EXPORT void
thunar_extension_shutdown (void)
{
    //#ifdef G_ENABLE_DEBUG
    g_message ("Shutting down thunar-nilfs2-plugin extension");
    //#endif
}



G_MODULE_EXPORT void
thunar_extension_list_types (const GType **types,
                             gint         *n_types)
{
    *types = type_list;
    *n_types = G_N_ELEMENTS (type_list);
}
