#include "../src/config.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <getopt.h>
#include <float.h>

#ifdef ENABLE_MIMALLOC
#  include <mimalloc-override.h>
#endif

#include <dwg.h>
#include <dwg_api.h>
#include "../src/geom.h"
#include <stdlib.h>
#include <string.h>

#define log_if_error(msg)                                                     \
  if (error)                                                                  \
    {                                                                         \
      fprintf (stderr, "ERROR: %s", msg);                                     \
      exit (1);                                                               \
    }
#define log_error(msg)                                                        \
  {                                                                           \
    fprintf (stderr, "ERROR: %s", msg);                                       \
    exit (1);                                                                 \
  }
#define dynget(obj, name, field, var)                                         \
  if (!dwg_dynapi_entity_value (obj, "" name, "" field, var, NULL))           \
    {                                                                         \
      fprintf (stderr, "ERROR: %s.%s", name, field);                          \
      exit (1);                                                               \
    }
#define dynget_utf8(obj, name, field, var)                                    \
  if (!dwg_dynapi_entity_utf8text (obj, "" name, "" field, var, &isnew,       \
                                   NULL))                                     \
    {                                                                         \
      fprintf (stderr, "ERROR: %s.%s", name, field);                          \
      exit (1);                                                               \
    }

double transform_X(double x);
double transform_Y(double y);

double model_xmin = DBL_MAX, model_ymin = DBL_MAX;

const uint8_t aci_to_rgb[256][3] = {
    {0, 0, 0},       // 0
    {255, 0, 0},       // 1
    {255, 255, 0},       // 2
    {0, 255, 0},       // 3
    {0, 255, 255},       // 4
    {0, 0, 255},       // 5
    {255, 0, 255},       // 6
    {255, 255, 255},       // 7
    {65, 65, 65},       // 8
    {128, 128, 128},       // 9
    {255, 0, 0},       // 10
    {255, 170, 170},       // 11
    {189, 0, 0},       // 12
    {189, 126, 126},       // 13
    {129, 0, 0},       // 14
    {129, 86, 86},       // 15
    {104, 0, 0},       // 16
    {104, 69, 69},       // 17
    {79, 0, 0},       // 18
    {79, 53, 53},       // 19
    {255, 63, 0},       // 20
    {255, 191, 170},       // 21
    {189, 46, 0},       // 22
    {189, 141, 126},       // 23
    {129, 31, 0},       // 24
    {129, 96, 86},       // 25
    {104, 25, 0},       // 26
    {104, 78, 69},       // 27
    {79, 19, 0},       // 28
    {79, 59, 53},       // 29
    {255, 127, 0},       // 30
    {255, 212, 170},       // 31
    {189, 94, 0},       // 32
    {189, 157, 126},       // 33
    {129, 64, 0},       // 34
    {129, 107, 86},       // 35
    {104, 52, 0},       // 36
    {104, 86, 69},       // 37
    {79, 39, 0},       // 38
    {79, 66, 53},       // 39
    {255, 191, 0},       // 40
    {255, 234, 170},       // 41
    {189, 141, 0},       // 42
    {189, 173, 126},       // 43
    {129, 96, 0},       // 44
    {129, 118, 86},       // 45
    {104, 78, 0},       // 46
    {104, 95, 69},       // 47
    {79, 59, 0},       // 48
    {79, 73, 53},       // 49
    {255, 255, 0},       // 50
    {255, 255, 170},       // 51
    {189, 189, 0},       // 52
    {189, 189, 126},       // 53
    {129, 129, 0},       // 54
    {129, 129, 86},       // 55
    {104, 104, 0},       // 56
    {104, 104, 69},       // 57
    {79, 79, 0},       // 58
    {79, 79, 53},       // 59
    {191, 255, 0},       // 60
    {234, 255, 170},       // 61
    {141, 189, 0},       // 62
    {173, 189, 126},       // 63
    {96, 129, 0},       // 64
    {118, 129, 86},       // 65
    {78, 104, 0},       // 66
    {95, 104, 69},       // 67
    {59, 79, 0},       // 68
    {73, 79, 53},       // 69
    {127, 255, 0},       // 70
    {212, 255, 170},       // 71
    {94, 189, 0},       // 72
    {157, 189, 126},       // 73
    {64, 129, 0},       // 74
    {107, 129, 86},       // 75
    {52, 104, 0},       // 76
    {86, 104, 69},       // 77
    {39, 79, 0},       // 78
    {66, 79, 53},       // 79
    {63, 255, 0},       // 80
    {191, 255, 170},       // 81
    {46, 189, 0},       // 82
    {141, 189, 126},       // 83
    {31, 129, 0},       // 84
    {96, 129, 86},       // 85
    {25, 104, 0},       // 86
    {78, 104, 69},       // 87
    {19, 79, 0},       // 88
    {59, 79, 53},       // 89
    {0, 255, 0},       // 90
    {170, 255, 170},       // 91
    {0, 189, 0},       // 92
    {126, 189, 126},       // 93
    {0, 129, 0},       // 94
    {86, 129, 86},       // 95
    {0, 104, 0},       // 96
    {69, 104, 69},       // 97
    {0, 79, 0},       // 98
    {53, 79, 53},       // 99
    {0, 255, 63},       // 100
    {170, 255, 191},       // 101
    {0, 189, 46},       // 102
    {126, 189, 141},       // 103
    {0, 129, 31},       // 104
    {86, 129, 96},       // 105
    {0, 104, 25},       // 106
    {69, 104, 78},       // 107
    {0, 79, 19},       // 108
    {53, 79, 59},       // 109
    {0, 255, 127},       // 110
    {170, 255, 212},       // 111
    {0, 189, 94},       // 112
    {126, 189, 157},       // 113
    {0, 129, 64},       // 114
    {86, 129, 107},       // 115
    {0, 104, 52},       // 116
    {69, 104, 86},       // 117
    {0, 79, 39},       // 118
    {53, 79, 66},       // 119
    {0, 255, 191},       // 120
    {170, 255, 234},       // 121
    {0, 189, 141},       // 122
    {126, 189, 173},       // 123
    {0, 129, 96},       // 124
    {86, 129, 118},       // 125
    {0, 104, 78},       // 126
    {69, 104, 95},       // 127
    {0, 79, 59},       // 128
    {53, 79, 73},       // 129
    {0, 255, 255},       // 130
    {170, 255, 255},       // 131
    {0, 189, 189},       // 132
    {126, 189, 189},       // 133
    {0, 129, 129},       // 134
    {86, 129, 129},       // 135
    {0, 104, 104},       // 136
    {69, 104, 104},       // 137
    {0, 79, 79},       // 138
    {53, 79, 79},       // 139
    {0, 191, 255},       // 140
    {170, 234, 255},       // 141
    {0, 141, 189},       // 142
    {126, 173, 189},       // 143
    {0, 96, 129},       // 144
    {86, 118, 129},       // 145
    {0, 78, 104},       // 146
    {69, 95, 104},       // 147
    {0, 59, 79},       // 148
    {53, 73, 79},       // 149
    {0, 127, 255},       // 150
    {170, 212, 255},       // 151
    {0, 94, 189},       // 152
    {126, 157, 189},       // 153
    {0, 64, 129},       // 154
    {86, 107, 129},       // 155
    {0, 52, 104},       // 156
    {69, 86, 104},       // 157
    {0, 39, 79},       // 158
    {53, 66, 79},       // 159
    {0, 63, 255},       // 160
    {170, 191, 255},       // 161
    {0, 46, 189},       // 162
    {126, 141, 189},       // 163
    {0, 31, 129},       // 164
    {86, 96, 129},       // 165
    {0, 25, 104},       // 166
    {69, 78, 104},       // 167
    {0, 19, 79},       // 168
    {53, 59, 79},       // 169
    {0, 0, 255},       // 170
    {170, 170, 255},       // 171
    {0, 0, 189},       // 172
    {126, 126, 189},       // 173
    {0, 0, 129},       // 174
    {86, 86, 129},       // 175
    {0, 0, 104},       // 176
    {69, 69, 104},       // 177
    {0, 0, 79},       // 178
    {53, 53, 79},       // 179
    {63, 0, 255},       // 180
    {191, 170, 255},       // 181
    {46, 0, 189},       // 182
    {141, 126, 189},       // 183
    {31, 0, 129},       // 184
    {96, 86, 129},       // 185
    {25, 0, 104},       // 186
    {78, 69, 104},       // 187
    {19, 0, 79},       // 188
    {59, 53, 79},       // 189
    {127, 0, 255},       // 190
    {212, 170, 255},       // 191
    {94, 0, 189},       // 192
    {157, 126, 189},       // 193
    {64, 0, 129},       // 194
    {107, 86, 129},       // 195
    {52, 0, 104},       // 196
    {86, 69, 104},       // 197
    {39, 0, 79},       // 198
    {66, 53, 79},       // 199
    {191, 0, 255},       // 200
    {234, 170, 255},       // 201
    {141, 0, 189},       // 202
    {173, 126, 189},       // 203
    {96, 0, 129},       // 204
    {118, 86, 129},       // 205
    {78, 0, 104},       // 206
    {95, 69, 104},       // 207
    {59, 0, 79},       // 208
    {73, 53, 79},       // 209
    {255, 0, 255},       // 210
    {255, 170, 255},       // 211
    {189, 0, 189},       // 212
    {189, 126, 189},       // 213
    {129, 0, 129},       // 214
    {129, 86, 129},       // 215
    {104, 0, 104},       // 216
    {104, 69, 104},       // 217
    {79, 0, 79},       // 218
    {79, 53, 79},       // 219
    {255, 0, 191},       // 220
    {255, 170, 234},       // 221
    {189, 0, 141},       // 222
    {189, 126, 173},       // 223
    {129, 0, 96},       // 224
    {129, 86, 118},       // 225
    {104, 0, 78},       // 226
    {104, 69, 95},       // 227
    {79, 0, 59},       // 228
    {79, 53, 73},       // 229
    {255, 0, 127},       // 230
    {255, 170, 212},       // 231
    {189, 0, 94},       // 232
    {189, 126, 157},       // 233
    {129, 0, 64},       // 234
    {129, 86, 107},       // 235
    {104, 0, 52},       // 236
    {104, 69, 86},       // 237
    {79, 0, 39},       // 238
    {79, 53, 66},       // 239
    {255, 0, 63},       // 240
    {255, 170, 191},       // 241
    {189, 0, 46},         // 242
    {189, 126, 141},      // 243
    {129, 0, 31},         // 244
    {129, 86, 96},        // 245
    {104, 0, 25},         // 246
    {104, 69, 78},        // 247
    {79, 0, 19},          // 248
    {79, 53, 59},         // 249
    {51, 51, 51},         // 250
    {80, 80, 80},         // 251
    {105, 105, 105},       // 252
    {130, 130, 130},       // 253
    {190, 190, 190},       // 254
    {255, 255, 255},       // 255
};

void get_rgb_color(dwg_object *obj, uint8_t *r, uint8_t *g, uint8_t *b) {
    int error;
    const Dwg_Color *color = dwg_ent_get_color(obj->tio.entity, &error);

    if (error)
        log_error("LINE.color");

    //printf("Color RGB value: %d\n", color->rgb);
    //printf("Color index: %d\n", color->index);

    if (color->rgb != 0) {
        *r = (color->rgb >> 16) & 0xFF;
        *g = (color->rgb >> 8) & 0xFF;
        *b = color->rgb & 0xFF;
    } else {
        if (color->index >= 0 && color->index < sizeof(aci_to_rgb) / sizeof(aci_to_rgb[0])) {
            *r = aci_to_rgb[color->index][0];
            *g = aci_to_rgb[color->index][1];
            *b = aci_to_rgb[color->index][2];
        } else {
            *r = *g = *b = 0; // Black color
        }
    }
}


char* dashes_to_svg_stroke_dasharray(Dwg_LTYPE_dash* dashes, BITCODE_RC numdashes) {
    char* result = malloc(numdashes * 2 * 20);
    result[0] = '\0'; 

    for (BITCODE_RC i = 0; i < numdashes; i++) {
        char buffer[20];
        sprintf(buffer, "%f,%f,", dashes[i].length, dashes[i].length);
        strcat(result, buffer);
    }

    result[strlen(result) - 1] = '\0';

    return result;
}



void calculate_min_coordinates(dwg_data *dwg) {
    for (BITCODE_BL i = 0; i < dwg->num_objects; i++) {
        dwg_object* obj = &dwg->object[i];
        if (obj->supertype == DWG_SUPERTYPE_ENTITY) {
            switch (dwg_object_get_type(obj)) {
                case DWG_TYPE_LINE: {
                    Dwg_Entity_LINE *line = dwg_object_to_LINE(obj);
                    if (line->start.x < model_xmin) model_xmin = line->start.x;
                    if (line->start.y < model_ymin) model_ymin = line->start.y;
                    if (line->end.x < model_xmin) model_xmin = line->end.x;
                    if (line->end.y < model_ymin) model_ymin = line->end.y;
                    break;
                }
                case DWG_TYPE_CIRCLE: {
                    Dwg_Entity_CIRCLE *circle = dwg_object_to_CIRCLE(obj);
                    double left = circle->center.x - circle->radius;
                    double right = circle->center.x + circle->radius;
                    double top = circle->center.y + circle->radius;
                    double bottom = circle->center.y - circle->radius;
                    if (left < model_xmin) model_xmin = left;
                    if (right < model_xmin) model_xmin = right;
                    if (top < model_ymin) model_ymin = top;
                    if (bottom < model_ymin) model_ymin = bottom;
                    break;
                }
                case DWG_TYPE_ARC: {
                    Dwg_Entity_ARC *arc = dwg_object_to_ARC(obj);
                    double left = arc->center.x - arc->radius;
                    double right = arc->center.x + arc->radius;
                    double top = arc->center.y + arc->radius;
                    double bottom = arc->center.y - arc->radius;
                    if (left < model_xmin) model_xmin = left;
                    if (right < model_xmin) model_xmin = right;
                    if (top < model_ymin) model_ymin = top;
                    if (bottom < model_ymin) model_ymin = bottom;
                    break;
                }
                case DWG_TYPE_VERTEX_2D: {
                    Dwg_Entity_VERTEX_2D *vertex = dwg_object_to_VERTEX_2D(obj);
                    if (vertex->point.x < model_xmin) model_xmin = vertex->point.x;
                    if (vertex->point.y < model_ymin) model_ymin = vertex->point.y;
                    break;
                }
                case DWG_TYPE_INSERT: {
                    Dwg_Entity_INSERT *insert = dwg_object_to_INSERT(obj);
                    if (insert->ins_pt.x < model_xmin) model_xmin = insert->ins_pt.x;
                    if (insert->ins_pt.y < model_ymin) model_ymin = insert->ins_pt.y;
                    break;
                }
                case DWG_TYPE_TEXT: {
                    Dwg_Entity_TEXT *text = dwg_object_to_TEXT(obj);
                    if (text->ins_pt.x < model_xmin) model_xmin = text->ins_pt.x;
                    if (text->ins_pt.y < model_ymin) model_ymin = text->ins_pt.y;
                    break;
                }
                default:
                    break;
            }
        }
    }
}

static void output_LINE(dwg_object *obj) {
    int error, index;
    Dwg_Entity_LINE *line;
    dwg_point_3d start, end;

    index = dwg_object_get_index(obj, &error);
    log_if_error("object_get_index");
    line = dwg_object_to_LINE(obj);
    if (!line)
        log_error("dwg_object_to_LINE");
    if (!dwg_get_LINE(line, "start", &start))
        log_error("LINE.start");
    if (!dwg_get_LINE(line, "end", &end))
        log_error("LINE.end");

    uint8_t r, g, b;
    get_rgb_color(obj, &r, &g, &b);

    Dwg_Object_Ref *linetype_ref = dwg_ent_get_ltype(obj->tio.entity, &error);
    Dwg_Object *linetype_obj;
    Dwg_Object_LTYPE *linetype;

    if (!linetype_ref) {
        //printf("LINE.linetype linetype_ref is NULL\n");
        // Use default linetype here
        linetype_obj = NULL;
        linetype = NULL;
    } else {
        printf("Linetype ref: %p\n", linetype_ref);
        linetype_obj = dwg_ref_get_object(linetype_ref, &error);
        if (!linetype_obj) {
            //printf("LINE.linetype linetype_obj is NULL\n");
            // Use default linetype here
            linetype = NULL;
        } else {
            //printf("Linetype obj: %p\n", linetype_obj);
            linetype = dwg_object_to_LTYPE(linetype_obj);
            if (!linetype) {
                //printf("LINE.linetype LTYPE *linetype is NULL\n");
                // Use default linetype here
            } else {
                //printf("Linetype: %p\n", linetype);
            }
        }
    }

    BITCODE_BS lineweight = dwg_ent_get_linewt(obj->tio.entity, &error);
    if (error)
        log_error("LINE.lineweight");

    char* stroke_dasharray = linetype ? dashes_to_svg_stroke_dasharray(linetype->dashes, linetype->numdashes) : NULL;

    printf("\t<path id=\"dwg-object-%d\" d=\"M %f,%f %f,%f\" "
           "style=\"fill:none;stroke:#%02X%02X%02X;stroke-width:%dpx;%s\" />\n",
           index, transform_X(start.x), transform_Y(start.y),
           transform_X(end.x), transform_Y(end.y),
           r, g, b, lineweight, stroke_dasharray ? stroke_dasharray : "");

    if (stroke_dasharray)
        free(stroke_dasharray);
}

static void output_POLYLINE_2D(dwg_object *obj) {
    Dwg_Entity_POLYLINE_2D *polyline;
    int error, index;

    index = dwg_object_get_index(obj, &error);
    log_if_error("object_get_index");
    polyline = dwg_object_to_POLYLINE_2D(obj);
    if (!polyline)
        log_error("dwg_object_to_POLYLINE_2D");

    uint8_t r, g, b;
    get_rgb_color(obj, &r, &g, &b);

    Dwg_Object_Ref *linetype_ref = dwg_ent_get_ltype(obj->tio.entity, &error);
    Dwg_Object *linetype_obj;
    Dwg_Object_LTYPE *linetype;

    if (!linetype_ref) {
        printf("POLYLINE_2D.linetype linetype_ref is NULL\n");
        // Use default linetype here
        linetype_obj = NULL;
        linetype = NULL;
    } else {
        printf("Linetype ref: %p\n", linetype_ref);
        linetype_obj = dwg_ref_get_object(linetype_ref, &error);
        if (!linetype_obj) {
            printf("POLYLINE_2D.linetype linetype_obj is NULL\n");
            // Use default linetype here
            linetype = NULL;
        } else {
            printf("Linetype obj: %p\n", linetype_obj);
            linetype = dwg_object_to_LTYPE(linetype_obj);
            if (!linetype) {
                printf("POLYLINE_2D.linetype LTYPE *linetype is NULL\n");
                // Use default linetype here
            } else {
                printf("Linetype: %p\n", linetype);
            }
        }
    }

    BITCODE_BS lineweight = dwg_ent_get_linewt(obj->tio.entity, &error);
    if (error)
        log_error("POLYLINE_2D.lineweight");

    char* stroke_dasharray = linetype ? dashes_to_svg_stroke_dasharray(linetype->dashes, linetype->numdashes) : NULL;

    printf("\t<path id=\"dwg-object-%d\" d=\"M ", index);
    for (BITCODE_BL i = 0; i < obj->parent->num_objects; i++) {
        dwg_object* vertex_obj = &obj->parent->object[i];
        if (vertex_obj->type == DWG_TYPE_VERTEX_2D) {
            Dwg_Entity_VERTEX_2D *vertex = dwg_object_to_VERTEX_2D(vertex_obj);
            if (vertex_obj->parent == obj->tio.entity) {
                printf("%f,%f ", transform_X(vertex->point.x), transform_Y(vertex->point.y));
            }
        }
    }
    printf("\" style=\"fill:none;stroke:#%02X%02X%02X;stroke-width:%dpx;%s\" />\n", r, g, b, lineweight, stroke_dasharray ? stroke_dasharray : "");

    if (stroke_dasharray)
        free(stroke_dasharray);
}


static double page_width, page_height, scale;



double
transform_X (double x)
{
  return x - model_xmin;
}

double
transform_Y (double y)
{
  return page_height - (y - model_ymin);
}

int layer_in_list(char** layers, int num_layers, const char* layer_name) {
    for (int i = 0; i < num_layers; i++) {
        if (strcmp(layers[i], layer_name) == 0) {
            return 1;
        }
    }
    return 0;
}

char** get_layers(dwg_data *dwg, int* num_layers) {
    char** layers = NULL;
    *num_layers = 0;

    for (BITCODE_BL i = 0; i < dwg->num_objects; i++) {
        dwg_object* obj = &dwg->object[i];
        if (obj->supertype == DWG_SUPERTYPE_ENTITY) {
            int error;
            char* layer_name = dwg_ent_get_layer_name(obj->tio.entity, &error);
            if (layer_name && !layer_in_list(layers, *num_layers, layer_name)) {
                layers = realloc(layers, (*num_layers + 1) * sizeof(char*));
                layers[*num_layers] = strdup(layer_name);
                (*num_layers)++;
            }
        }
    }

    return layers;
}

char* escape_svg(const char* input) {
    size_t length = strlen(input);
    size_t extra_chars = 0;

    for (size_t i = 0; i < length; i++) {
        if (input[i] == '<' || input[i] == '>' || input[i] == '&') {
            extra_chars += 4; // < or > or &
        }
    }

    char* output = malloc(length + extra_chars + 1);
    size_t j = 0;

    for (size_t i = 0; i < length; i++) {
        if (input[i] == '<') {
            strcpy(&output[j], "<");
            j += 4;
        } else if (input[i] == '>') {
            strcpy(&output[j], ">");
            j += 4;
        } else if (input[i] == '&') {
            strcpy(&output[j], "&");
            j += 5;
        } else {
            output[j] = input[i];
            j++;
        }
    }

    output[j] = '\0';

    return output;
}

static void output_CIRCLE (dwg_object *obj)
{
  Dwg_Entity_CIRCLE *circle;
  int error, index;
  double radius;
  dwg_point_3d center;

  index = dwg_object_get_index (obj, &error);
  log_if_error ("object_get_index");
  circle = dwg_object_to_CIRCLE (obj);
  if (!circle)
    log_error ("dwg_object_to_CIRCLE");
  if (!dwg_get_CIRCLE (circle, "center", &center))
    log_error ("CIRCLE.center");
  if (!dwg_get_CIRCLE (circle, "radius", &radius))
    log_error ("CIRCLE.radius");

  
  uint8_t r, g, b;
  get_rgb_color(obj, &r, &g, &b);


  printf ("\t<circle id=\"dwg-object-%d\" cx=\"%f\" cy=\"%f\" r=\"%f\" "
          "fill=\"none\" stroke=\"#%02X%02X%02X\" stroke-width=\"1px\" />\n",
          index, transform_X (center.x), transform_Y (center.y), radius,
          r, g, b); // Use the RGB color value
}

static void output_ARC (dwg_object *obj)
{
  Dwg_Entity_ARC *arc;
  int error, index;
  double radius, start_angle, end_angle;
  dwg_point_3d center;
  double x_start, y_start, x_end, y_end;
  int large_arc;

  index = dwg_object_get_index (obj, &error);
  log_if_error ("object_get_index");
  arc = dwg_object_to_ARC (obj);
  if (!arc)
    log_error ("dwg_object_to_ARC");
  dynget (arc, "ARC", "radius", &radius);
  dynget (arc, "ARC", "center", &center);
  dynget (arc, "ARC", "start_angle", &start_angle);
  dynget (arc, "ARC", "end_angle", &end_angle);

  
  uint8_t r, g, b;
  get_rgb_color(obj, &r, &g, &b);

  x_start = center.x + radius * cos (start_angle);
  y_start = center.y + radius * sin (start_angle);
  x_end = center.x + radius * cos (end_angle);
  y_end = center.y + radius * sin (end_angle);
  large_arc = (end_angle - start_angle < M_PI) ? 0 : 1;

  printf ("\t<path id=\"dwg-object-%d\" d=\"M %f,%f A %f,%f 0 %d 0 %f,%f\" "
          "fill=\"none\" stroke=\"#%02X%02X%02X\" stroke-width=\"1px\" />\n",
          index, transform_X (x_start), transform_Y (y_start), radius, radius,
          large_arc, transform_X (x_end), transform_Y (y_end),
          r, g, b); // Use the RGB color value
}


static void output_TEXT (dwg_object *obj)
{
  int error, index;
  dwg_point_2d ins_pt;
  Dwg_Entity_TEXT *text;
  char *text_value;
  double fontsize;
  const Dwg_Version_Type dwg_version = obj->parent->header.version;
  int isnew = 0;

  index = dwg_object_get_index (obj, &error);
  log_if_error ("object_get_index");
  text = dwg_object_to_TEXT (obj);
  if (!text)
    log_error ("dwg_object_to_TEXT");
  dynget_utf8 (text, "TEXT", "text_value", &text_value);
  dynget (text, "TEXT", "ins_pt", &ins_pt);
  dynget (text, "TEXT", "height", &fontsize);

  
  uint8_t r, g, b;
  get_rgb_color(obj, &r, &g, &b);

  char *escaped_text_value = escape_svg(text_value);
  printf ("\t<text id=\"dwg-object-%d\" x=\"%f\" y=\"%f\" "
          "font-family=\"Verdana\" font-size=\"%f\" fill=\"#%02X%02X%02X\">%s</text>\n",
          index, transform_X (ins_pt.x), transform_Y (ins_pt.y), fontsize,
          r, g, b, escaped_text_value); // Use the RGB color value

  if (text_value && isnew)
    free (text_value);
}

static void output_INSERT (dwg_object *obj)
{
  int index, error;
  BITCODE_RL abs_ref;
  double rotation;
  dwg_ent_insert *insert;
  dwg_point_3d ins_pt, _scale;
  dwg_handle *obj_handle, *ins_handle;
  Dwg_Data *dwg;

  insert = dwg_object_to_INSERT (obj);
  if (!insert)
    log_error ("dwg_object_to_INSERT");
  dwg = obj->parent;
  index = dwg_object_get_index (obj, &error);
  log_if_error ("object_get_index");
  dynget (insert, "INSERT", "rotation", &rotation);
  dynget (insert, "INSERT", "ins_pt", &ins_pt);
  dynget (insert, "INSERT", "scale", &_scale);
  obj_handle = dwg_object_get_handle (obj, &error);
  log_if_error ("get_handle");
  if (!insert->block_header)
    log_error ("insert->block_header");
  abs_ref = insert->block_header->absolute_ref;

  
  uint8_t r, g, b;
  get_rgb_color(obj, &r, &g, &b);

  if (insert->block_header->handleref.code == 5 || dwg->header.version < R_13)
    {
      printf ("\t<use id=\"dwg-object-%d\" transform=\"translate(%f %f) "
              "rotate(%f) scale(%f %f)\" xlink:href=\"#symbol-%X\" fill=\"#%02X%02X%02X\" />\n",
              index, transform_X (ins_pt.x), transform_Y (ins_pt.y),
              (180.0 / M_PI) * rotation, _scale.x, _scale.y, abs_ref,
              r, g, b); // Use the RGB color value
    }
  else
    {
      printf ("\n\n<!-- WRONG INSERT(" FORMAT_H ") -->\n",
              ARGS_H (*obj_handle));
    }
}

void output_SVG_for_layer(dwg_data *dwg, const char* layer_name) {
    unsigned int i;
    dwg_object *obj;

    for (i = 0; i < dwg->num_objects; i++) {
        obj = &dwg->object[i];
        if (obj->supertype == DWG_SUPERTYPE_ENTITY) {
            int error;
            char* entity_layer_name = dwg_ent_get_layer_name(obj->tio.entity, &error);
            if (entity_layer_name && strcmp(entity_layer_name, layer_name) == 0) {
                switch (dwg_object_get_fixedtype(obj)) {
                    case DWG_TYPE_LINE:
                        output_LINE(obj);
                        break;
                    case DWG_TYPE_CIRCLE:
                        output_CIRCLE(obj);
                        break;
                    case DWG_TYPE_TEXT:
                        output_TEXT(obj);
                        break;
                    case DWG_TYPE_ARC:
                        output_ARC(obj);
                        break;
                    case DWG_TYPE_INSERT:
                        output_INSERT(obj);
                        break;
                    case DWG_TYPE_POLYLINE_2D:
                        output_POLYLINE_2D(obj);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int c;
    char* filename = NULL;
    char* layer_name = NULL;

    while ((c = getopt(argc, argv, "f:l:")) != -1) {
        switch (c) {
            case 'f':
                filename = optarg;
                break;
            case 'l':
                layer_name = optarg;
                break;
            default:
                fprintf(stderr, "Invalid option\n");
                return 1;
        }
    }

    if (!filename) {
        fprintf(stderr, "No filename provided\n");
        return 1;
    }

    dwg_data g_dwg = {0};

    int error = dwg_read_file(filename, &g_dwg);
    if (error >= DWG_ERR_CRITICAL) {
        fprintf(stderr, "Failed to read DWG file: %s\n", filename);
        fprintf(stderr, "Failed to read DWG error: %d\n", error);
        return 1;
    }

    calculate_min_coordinates(&g_dwg);

    if (layer_name) {
        output_SVG_for_layer(&g_dwg, layer_name);
    } else {
        int num_layers;
        char** layers = get_layers(&g_dwg, &num_layers);
        if (!layers) {
            fprintf(stderr, "Failed to get layers\n");
            dwg_free(&g_dwg);
            return 1;
        }

        for (int layer_index = 0; layer_index < num_layers; layer_index++) {
            printf("%s\n", layers[layer_index]);
        }

        for (int layer_index = 0; layer_index < num_layers; layer_index++) {
            free(layers[layer_index]);
        }
        free(layers);
    }

    dwg_free(&g_dwg);

    return 0;
}
