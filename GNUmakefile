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
MODULES += src/jsoncpp
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
# CLion Integration: Convenience targets for building specific platform/variant
# combinations. These don't change the default behavior - GitHub Actions still
# uses 'PLATFORM=linux64 make release' as before.
###############################################################################

.PHONY: linux-release linux-debug linux64-release linux64-debug
.PHONY: mingw-release mingw-debug mingw64-release mingw64-debug

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
