ifeq ($(PROJECT),)
   PROJECT = .
endif

ifeq ($(PLATFORM),)
    ifeq ($(VARIANT),)
        BUILD/ = $(PROJECT/)build/
    else
        BUILD/ = $(PROJECT/)build-$(VARIANT)/
    endif
else
    ifeq ($(VARIANT),)
        BUILD/ = $(PROJECT/)build.$(PLATFORM)/
    else
        BUILD/ = $(PROJECT/)build.$(PLATFORM)-$(VARIANT)/
    endif
endif

###############################################################################

MODULES += src/lua
MODULES += src/sqlite3
MODULES += src/base
MODULES += src/cgame
MODULES += src/game
MODULES += src/ui
MODULES += pak/pak
MODULES += pkg/pkg
MODULES += doc/book

###############################################################################

# Default goal depends on whether PLATFORM is set or not
# If PLATFORM is set, default to pkg (build single platform)
# If PLATFORM is not set, default to build-all (multi-platform via script)
ifdef PLATFORM
.DEFAULT_GOAL := pkg
else
.DEFAULT_GOAL := build-all
endif

###############################################################################

-include $(BUILD/)make/project.mk

ifdef PROJECT.name
include $(PROJECT)/make/main.mk
endif

###############################################################################

$(BUILD/)make/project.mk: $(PROJECT/)project/info.py $(PROJECT/)project/info.db
	$(call print.HEADER,GENERATING,$@)
	@mkdir -p $(dir $@)
	@$< -mk $(PROJECT/)project/info.db > $@

###############################################################################

# Prevent recursive make from running build-all.sh again
# Only define build-all target when PLATFORM is not set and we're not called from the script
ifneq ($(XMOD_BUILD_SCRIPT),1)
ifndef PLATFORM

.PHONY: build-all all-platforms
build-all all-platforms:
	@chmod +x $(PROJECT/)build-all.sh
	@export XMOD_BUILD_SCRIPT=1 && $(PROJECT/)build-all.sh $(if $(VARIANT),$(VARIANT),release) $(if $(NO_CLEAN),--no-clean,$(if $(CLEAN),--clean,))

endif
endif

###############################################################################

# Individual platform build targets for CLion integration
.PHONY: linux-release linux-debug linux64-release linux64-debug
.PHONY: mingw-release mingw-debug mingw64-release mingw64-debug
.PHONY: android-arm64-release android-arm64-debug
.PHONY: android-x86_64-release android-x86_64-debug
.PHONY: android-x86-release android-x86-debug

linux-release:
	$(MAKE) PLATFORM=linux VARIANT=release pkg

linux-debug:
	$(MAKE) PLATFORM=linux VARIANT=debug pkg

linux64-release:
	$(MAKE) PLATFORM=linux64 VARIANT=release pkg

linux64-debug:
	$(MAKE) PLATFORM=linux64 VARIANT=debug pkg

mingw-release:
	$(MAKE) PLATFORM=mingw VARIANT=release pkg

mingw-debug:
	$(MAKE) PLATFORM=mingw VARIANT=debug pkg

mingw64-release:
	$(MAKE) PLATFORM=mingw64 VARIANT=release pkg

mingw64-debug:
	$(MAKE) PLATFORM=mingw64 VARIANT=debug pkg

android-arm64-release:
	$(MAKE) PLATFORM=android-arm64 VARIANT=release pkg

android-arm64-debug:
	$(MAKE) PLATFORM=android-arm64 VARIANT=debug pkg

android-x86_64-release:
	$(MAKE) PLATFORM=android-x86_64 VARIANT=release pkg

android-x86_64-debug:
	$(MAKE) PLATFORM=android-x86_64 VARIANT=debug pkg

android-x86-release:
	$(MAKE) PLATFORM=android-x86 VARIANT=release pkg

android-x86-debug:
	$(MAKE) PLATFORM=android-x86 VARIANT=debug pkg

