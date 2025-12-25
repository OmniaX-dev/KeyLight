# other/Gettext.cmake

# Find required tools
find_package(Gettext REQUIRED)
find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
find_program(GETTEXT_MSGFMT_EXECUTABLE msgfmt)
find_program(GETTEXT_MSGMERGE_EXECUTABLE msgmerge)

if(NOT GETTEXT_XGETTEXT_EXECUTABLE OR NOT GETTEXT_MSGFMT_EXECUTABLE OR NOT GETTEXT_MSGMERGE_EXECUTABLE)
	message(FATAL_ERROR "gettext tools (xgettext/msgfmt/msgmerge) not found! Install gettext.")
endif()

# === Configurable variables (set before include) ===
# GETTEXT_DOMAIN          - text domain (default: project name)
# GETTEXT_LOCALEDIR       - runtime locale dir (default: ${CMAKE_INSTALL_PREFIX}/share/locale)
# GETTEXT_SOURCES         - list of source files to scan
# GETTEXT_LANGUAGES       - list of languages (e.g., es fr de)

if(NOT GETTEXT_DOMAIN)
	set(GETTEXT_DOMAIN ${PROJECT_NAME})
endif()

if(NOT GETTEXT_LOCALEDIR)
	set(GETTEXT_LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
endif()

if(NOT GETTEXT_SOURCES)
	message(FATAL_ERROR "GETTEXT_SOURCES must be set before including Gettext.cmake")
endif()

if(NOT GETTEXT_LANGUAGES)
	set(GETTEXT_LANGUAGES en)  # fallback
endif()

# === Generate .pot file ===
set(POT_FILE "${CMAKE_CURRENT_LIST_DIR}/../po/${GETTEXT_DOMAIN}.pot")

add_custom_command(
				OUTPUT ${POT_FILE}
				COMMAND ${GETTEXT_XGETTEXT_EXECUTABLE}
								--c++ --keyword=_ --keyword=ngettext:1,2
								--add-comments=TRANSLATORS
								--package-name=${PROJECT_NAME}
								--package-version=${PROJECT_VERSION}
								--msgid-bugs-address="syl4r39@gmail.com"
								--output=${POT_FILE}
								${GETTEXT_SOURCES}
				DEPENDS ${GETTEXT_SOURCES}
				COMMENT "Generating ${GETTEXT_DOMAIN}.pot"
				VERBATIM
)

# === Compile .po → .mo and auto-update with msgmerge ===
set(MO_FILES "")
foreach(LANG ${GETTEXT_LANGUAGES})
	set(PO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/po/${LANG}.po")
	set(MO_DIR "${CMAKE_CURRENT_BINARY_DIR}/locale/${LANG}/LC_MESSAGES")
	set(MO_FILE "${MO_DIR}/${GETTEXT_DOMAIN}.mo")
	set(STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${LANG}.po.stamp")

	# 1. msgmerge: Update .po → create stamp file
	add_custom_command(
								OUTPUT ${STAMP_FILE}
								COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE}
												--update --backup=off "${PO_FILE}" "${POT_FILE}"
								COMMAND ${CMAKE_COMMAND} -E touch ${STAMP_FILE}
								DEPENDS ${POT_FILE} ${PO_FILE}
								COMMENT "Updating ${LANG}.po with msgmerge"
								VERBATIM
				)

	# 2. msgfmt: Compile .po → .mo
	add_custom_command(
								OUTPUT ${MO_FILE}
								COMMAND ${CMAKE_COMMAND} -E make_directory "${MO_DIR}"
								COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "${PO_FILE}" -o "${MO_FILE}"
								DEPENDS ${STAMP_FILE}
								COMMENT "Compiling ${LANG}.po → .mo"
								VERBATIM
				)

	# Optional: Clean up stamp file (runs on 'make clean')
	set_property(DIRECTORY APPEND PROPERTY
								ADDITIONAL_CLEAN_FILES ${STAMP_FILE}
				)

	list(APPEND MO_FILES ${MO_FILE})
	list(APPEND STAMP_FILES ${STAMP_FILE})
endforeach()

# Optional: Clean all stamps on 'make clean'
set_property(DIRECTORY APPEND PROPERTY
				ADDITIONAL_CLEAN_FILES ${STAMP_FILES}
)

# === Custom target ===
add_custom_target(translations ALL DEPENDS ${MO_FILES} ${POT_FILE})

# === Pass to code ===
target_compile_definitions(${PROJECT_NAME} PRIVATE
				GETTEXT_DOMAIN="${GETTEXT_DOMAIN}"
				GETTEXT_LOCALEDIR="${GETTEXT_LOCALEDIR}"
)
