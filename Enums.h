#ifndef ENUMS_H
#define ENUMS_H

enum class eTheme {
    System,
    Classic,
    Dark,
    OLED,
    MidnightPurple
};

enum class eLanguage {
    English,
    French
};

enum class eLog {
    Notice,
    Success,
    Warning,
    Error
};

// Changing the order will break databases
enum class eViewType {
    Bluray = 0,
    Television = 1,
    Cinema = 2,
    CinemaReplay = 3,
    DVD = 4,
    Streaming = 5,
    Download = 6,
    VHS = 7,
    Other = 8,
    Unknown = 9,
    UHD = 10,
    MaxViewType = 11
};

enum class eChart {
    ViewedByMovieYear,
    ViewedByType,
    ViewedByYear
};

enum class eColumnType {
    Integer = 0,
    Double = 1,
    Text = 2
};

#endif // ENUMS_H
