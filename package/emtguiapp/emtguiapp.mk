EMTGUIAPP_SITE =  $(BASE_DIR)/../package/emtguiapp
EMTGUIAPP_SITE_METHOD = local
EMTGUIAPP_INSTALL_TARGET = YES
EMTGUIAPP_DEPENDENCIES = qt

define EMTGUIAPP_CONFIGURE_CMDS
    (cd $(@D); $(TARGET_MAKE_ENV) $(QT_QMAKE))
endef

define EMTGUIAPP_BUILD_CMDS
    $(TARGET_MAKE_ENV) $(MAKE) -C $(@D)
endef

define EMTGUIAPP_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/emtguiapp $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
