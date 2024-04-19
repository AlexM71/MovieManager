!versionAtLeast(QT_VERSION, 6.7.0) {
    error("You need to use atleast Qt 6.7.0 (current: $${QT_VERSION})")
}

SOURCES += \
    Common.cpp \
    CustomColumnLineEdit.cpp \
    Dialogs/AboutDialog.cpp \
    Dialogs/AddColumnDialog.cpp \
    Dialogs/AddViewDialog.cpp \
    Dialogs/CalendarDialog.cpp \
    Dialogs/ChangelogDialog.cpp \
    Dialogs/ChartsDialog.cpp \
    Dialogs/EditMovieDialog.cpp \
    Dialogs/EditViewDialog.cpp \
    Dialogs/EditViewsDialog.cpp \
    Dialogs/FiltersDialog.cpp \
    Dialogs/LogDialog.cpp \
    Dialogs/OptionsDialog.cpp \
    Log.cpp \
    MainWindow.cpp \
    Tag.cpp \
    TagsScrollArea.cpp \
    main.cpp

HEADERS += \
    BuildFunctions.h \
    Common.h \
    CustomColumnLineEdit.h \
    Dialogs/AboutDialog.h \
    Dialogs/AddColumnDialog.h \
    Dialogs/AddViewDialog.h \
    Dialogs/CalendarDialog.h \
    Dialogs/ChangelogDialog.h \
    Dialogs/ChartsDialog.h \
    Dialogs/EditMovieDialog.h \
    Dialogs/EditViewDialog.h \
    Dialogs/EditViewsDialog.h \
    Dialogs/FiltersDialog.h \
    Dialogs/LogDialog.h \
    Dialogs/OptionsDialog.h \
    Enums.h \
    Log.h \
    MainWindow.h \
    Structures.h \
    Tag.h \
    TagsScrollArea.h

QT += widgets \
      sql \
      charts \
      network \
      xml

FORMS += \
    Dialogs/AboutDialog.ui \
    Dialogs/AddColumnDialog.ui \
    Dialogs/AddViewDialog.ui \
    Dialogs/CalendarDialog.ui \
    Dialogs/ChangelogDialog.ui \
    Dialogs/ChartsDialog.ui \
    Dialogs/EditMovieDialog.ui \
    Dialogs/EditViewDialog.ui \
    Dialogs/EditViewsDialog.ui \
    Dialogs/FiltersDialog.ui \
    Dialogs/LogDialog.ui \
    Dialogs/OptionsDialog.ui \
    MainWindow.ui

RESOURCES += \
    MovieManager.qrc

TRANSLATIONS += Localisation/MovieManager_fr_FR.ts \
                Localisation/MovieManager_en_US.ts

RC_ICONS += Assets/logo.ico
