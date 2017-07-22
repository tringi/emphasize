#ifndef WINDOWS_CTRLIDS_HPP
#define WINDOWS_CTRLIDS_HPP

/* Windows CtrlIDs 
// Windows_CtrlIDs.hpp
//
// Author: Jan Ringos, http://Tringi.MX-3.cz, jan@ringos.cz
// Version: 0.3
//
// Description:
//      Allows automatic ID assignment by simply prefixing low ordinals with
//      special token (defined macros below) and minus (as if it is a dash).
//
// Changelog:
//      16.01.2013 - initial version
//      08.10.2014 - added static asserts for number of controls
//
// Resource file usage:
//      That is:    "BUTTON-3" is converted to 32- -3 => 32+3
//      For labels: BUTTON-3-LABEL
//      Grouping:   GROUP(2)-BUTTON-3
//
// C++ file usage:
//      Simplest:   ID::BUTTON-3 or ID::BUTTON[3] (both are equivalent)
//      Grouping:   ID::GROUP[2] | ID::BUTTON-3
// 
// Ranges:
//      Group numbers: 0 ... 30, group 0 is NOT the same as no group (-1)
*/

// default number of IDs

#define EMPHASIZE_WINDOWS_CTRL_ID_BASE              0x0020  // Below are IDOK...
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BUTTON
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BUTTON  128
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_STATIC
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_STATIC  192
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_INPUT
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_INPUT   256
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LIST
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LIST    64
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_VIEW
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_VIEW    64
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BAR
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BAR     32
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_TABS
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_TABS    32
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ANIMATION
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ANIMATION 32
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LINK
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LINK    32
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_HEADING
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_HEADING 32
#endif
#ifndef EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_DIVIDER
#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_DIVIDER 32
#endif

#define EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_GROUPS  30

// base ids
//  - comment says how many we actually need
//  - don't mind the long prefix

#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_BUTTON       (EMPHASIZE_WINDOWS_CTRL_ID_BASE)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_STATIC       (EMPHASIZE_WINDOWS_CTRL_ID_BASE_BUTTON + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BUTTON)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_INPUT        (EMPHASIZE_WINDOWS_CTRL_ID_BASE_STATIC + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_STATIC)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_LIST         (EMPHASIZE_WINDOWS_CTRL_ID_BASE_INPUT + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_INPUT)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_VIEW         (EMPHASIZE_WINDOWS_CTRL_ID_BASE_LIST + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LIST)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_BAR          (EMPHASIZE_WINDOWS_CTRL_ID_BASE_VIEW + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_VIEW)

#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_TABS         (EMPHASIZE_WINDOWS_CTRL_ID_BASE_BAR + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_BAR)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_ANIMATION    (EMPHASIZE_WINDOWS_CTRL_ID_BASE_TABS + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_TABS)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_LINK         (EMPHASIZE_WINDOWS_CTRL_ID_BASE_ANIMATION + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ANIMATION)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_HEADING      (EMPHASIZE_WINDOWS_CTRL_ID_BASE_LINK + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_LINK)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_DIVIDER      (EMPHASIZE_WINDOWS_CTRL_ID_BASE_HEADING + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_HEADING)

// additional
//  - group base (multiplier)
//  - label bit
//  - snap invisible controls

#define EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE         0x03FF
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_GROUP        0x0400  // 30 groups
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_SNAP         0x7C00  // 256 snaps :)
#define EMPHASIZE_WINDOWS_CTRL_ID_BASE_LABEL        0x8000  // 1 bit (0/1)

// static checks

#ifndef RC_INVOKED
static_assert ((EMPHASIZE_WINDOWS_CTRL_ID_BASE_DIVIDER + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_DIVIDER)
               <= (EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE + 1u),
               "EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ defines overflow! Some is set too high. IDs would overlap next group.");
#endif

// introduce

#ifdef RC_INVOKED
// version for .rc files
#define BUTTON      EMPHASIZE_WINDOWS_CTRL_ID_BASE_BUTTON-
#define INPUT       EMPHASIZE_WINDOWS_CTRL_ID_BASE_INPUT-
#define STATIC      EMPHASIZE_WINDOWS_CTRL_ID_BASE_STATIC-
#define LIST        EMPHASIZE_WINDOWS_CTRL_ID_BASE_LIST-
#define VIEW        EMPHASIZE_WINDOWS_CTRL_ID_BASE_VIEW-
#define BAR         EMPHASIZE_WINDOWS_CTRL_ID_BASE_BAR-

#define TABS        EMPHASIZE_WINDOWS_CTRL_ID_BASE_TABS-
#define ANIMATION   EMPHASIZE_WINDOWS_CTRL_ID_BASE_ANIMATION-
#define LINK        EMPHASIZE_WINDOWS_CTRL_ID_BASE_LINK-
#define HEADING     EMPHASIZE_WINDOWS_CTRL_ID_BASE_HEADING-
#define DIVIDER     EMPHASIZE_WINDOWS_CTRL_ID_BASE_DIVIDER-

#define SNAP        EMPHASIZE_WINDOWS_CTRL_ID_BASE_SNAP
#define LABEL       EMPHASIZE_WINDOWS_CTRL_ID_BASE_LABEL
#define GROUP(g)    ((g + 1) * EMPHASIZE_WINDOWS_CTRL_ID_BASE_GROUP)-
#else

// version for C++ source/header files
//  - in uppercase to maintain visual consistency with base api declarations

#define DEFINE_ACCESSOR(name) \
    static constexpr struct name ## _type {                     \
        constexpr int operator [] (int i) const {               \
            return EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name + i; \
        };                                                      \
        constexpr int operator - (int i) const {                \
            return EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name + i; \
        };                                                      \
    } name /*__attribute__((__unused__))*/ = {}

#define DEFINE_IS_ID_QUERY() \
    static constexpr bool IsID (int id) {                                                   \
        return (id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) < EMPHASIZE_WINDOWS_CTRL_ID_BASE; \
    }
#define DEFINE_IS_QUERY(name) \
    static constexpr bool Is ## name (int id) {                                                             \
        return (id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) >= EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name        \
            && (id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) < EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name         \
                                                          + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ ## name;   \
    }
    
#define DEFINE_ACCESSORS() \
    DEFINE_IS_ID_QUERY ();      \
    DEFINE_ACCESSOR (BUTTON);   \
    DEFINE_IS_QUERY (BUTTON);   \
    DEFINE_ACCESSOR (INPUT);    \
    DEFINE_IS_QUERY (INPUT);    \
    DEFINE_ACCESSOR (STATIC);   \
    DEFINE_IS_QUERY (STATIC);   \
    DEFINE_ACCESSOR (LIST);     \
    DEFINE_IS_QUERY (LIST);     \
    DEFINE_ACCESSOR (VIEW);     \
    DEFINE_IS_QUERY (VIEW);     \
    DEFINE_ACCESSOR (BAR);      \
    DEFINE_IS_QUERY (BAR);      \
    DEFINE_ACCESSOR (TABS);     \
    DEFINE_IS_QUERY (TABS);     \
    DEFINE_ACCESSOR (ANIMATION);\
    DEFINE_IS_QUERY (ANIMATION);\
    DEFINE_ACCESSOR (LINK);     \
    DEFINE_IS_QUERY (LINK);     \
    DEFINE_ACCESSOR (HEADING);  \
    DEFINE_IS_QUERY (HEADING);  \
    DEFINE_ACCESSOR (DIVIDER);  \
    DEFINE_IS_QUERY (DIVIDER);  \
    DEFINE_ACCESSOR (SNAP)

#define DEFINE_INDEXOF(id,name) \
    ((id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) < (EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name + EMPHASIZE_WINDOWS_CTRL_ID_NUMBER_OF_ ## name)) \
        ? ((id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) - EMPHASIZE_WINDOWS_CTRL_ID_BASE_ ## name)

namespace ID {
    DEFINE_ACCESSORS ();
    
    static const int LABEL = EMPHASIZE_WINDOWS_CTRL_ID_BASE_LABEL;
    static constexpr struct GROUP_ {
        constexpr int operator [] (int i) const {
            // assert i range?
            return (i + 1) * EMPHASIZE_WINDOWS_CTRL_ID_BASE_GROUP;
        };
        constexpr int operator - (int i) const {
            // assert i range?
            return (i + 1) * EMPHASIZE_WINDOWS_CTRL_ID_BASE_GROUP;
        };
    } GROUP /*__attribute__((__unused__))*/ = {};

    constexpr int GroupOf (unsigned int id) {
        return signed (id & ~EMPHASIZE_WINDOWS_CTRL_ID_BASE_LABEL)
             / EMPHASIZE_WINDOWS_CTRL_ID_BASE_GROUP
             - 1;
    };
    constexpr unsigned int IndexOf (unsigned int id) {
        return ((id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE) < EMPHASIZE_WINDOWS_CTRL_ID_BASE)
             ? (id & EMPHASIZE_WINDOWS_CTRL_ID_MASK_BASE)
             : DEFINE_INDEXOF (id, BUTTON)
             : DEFINE_INDEXOF (id, INPUT)
             : DEFINE_INDEXOF (id, STATIC)
             : DEFINE_INDEXOF (id, LIST)
             : DEFINE_INDEXOF (id, VIEW)
             : DEFINE_INDEXOF (id, BAR)
             : DEFINE_INDEXOF (id, TABS)
             : DEFINE_INDEXOF (id, ANIMATION)
             : DEFINE_INDEXOF (id, LINK)
             : DEFINE_INDEXOF (id, HEADING)
             : DEFINE_INDEXOF (id, DIVIDER)
             : -1;
    };
};

#undef DEFINE_INDEXOF
#undef DEFINE_ACCESSORS
#undef DEFINE_ACCESSOR

#endif
#endif

