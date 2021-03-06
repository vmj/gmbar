#ifndef LIBGMBAR_H
#define LIBGMBAR_H

/**
 * Structure to represent size.
 */
typedef struct gmsize gmsize;
struct gmsize {
        /** Width */
        unsigned int width;
        /** Heights */
        unsigned int height;
};

/**
 * Structure to represent margins and paddings.
 */
typedef struct gmmargin gmmargin;
struct gmmargin {
        /** Top margin */
        unsigned char top;
        /** Right margin */
        unsigned char right;
        /** Bottom margin */
        unsigned char bottom;
        /** Left margin */
        unsigned char left;
};

/**
 * Structure to represent color.
 */
typedef struct gmcolor gmcolor;
struct gmcolor {
        /** Foreground colod */
        char* fg;
        /** Background color */
        char* bg;
};

/**
 * Structure to represent a section in the gmbar.
 */
typedef struct gmsection gmsection;
struct gmsection {
        /** Container */
        void* bar;
        /** Width of the section */
        unsigned int width;
        /** Color of the section */
        char* color;
};

/**
 * Structure to represent graphical multi bar.
 */
typedef struct gmbar gmbar;
struct gmbar {
        /** Bar size, including margins */
        gmsize size;
        /** Bar color */
        gmcolor color;
        /** Bar margins */
        gmmargin margin;
        /** Bar paddings */
        gmmargin padding;
        /** Segment width */
        unsigned int segment_width;
        /** Segment gap width */
        unsigned int segment_gap;
        /** Section width granularity */
        unsigned int granularity;
        /** Section width rounding */
        double rounding;
        /** Number of sections */
        unsigned char nsections;
        /** List of sections */
        gmsection** sections;
};


gmbar*           gmbar_new                    ();
gmbar*           gmbar_new_with_defaults      (unsigned int width,
                                               unsigned int height,
                                               char* fg,
                                               char* bg);
void             gmbar_free                   (gmbar* bar);

int              gmbar_add_section            (gmbar* bar,
                                               char* color);
int              gmbar_add_sections           (gmbar* bar,
                                               unsigned int nsections,
                                               ...);
void             gmbar_set_section_width      (gmsection* section,
                                               unsigned int total,
                                               unsigned int value);

int              gmbar_format                 (gmbar* bar,
                                               unsigned int nl,
                                               char** buf,
                                               int* len,
                                               int* max);

#endif // LIBGMBAR_H
