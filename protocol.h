#include <stdlib.h>

#include <furi.h>
#include <cli/cli.h>
#include <input/input.h>
#include <gui/gui.h>

#define CNT_FLIPPER_START_ID            0x0001
#define CNT_PYTHON_START_ID             0x0002
#define CNT_FLIPPER_STOP_ID             0xFFF1
#define CNT_PYTHON_STOP_ID              0xFFF2

#define INPUT_ID                            0x1001

#define GUI_DRAW_ID                         0x2000
#define GUI_DRAW_STR_ID                     0x2001
#define GUI_DRAW_STR_ALIGN_ID               0x2002
#define GUI_DRAW_FRAME_ID                   0x2003
#define GUI_DRAW_RFRAME_ID                  0x2004
#define GUI_DRAW_ICON_ID                    0x2005
#define GUI_DRAW_DOT_ID                     0x2006
#define GUI_DRAW_LINE_ID                    0x2007
#define GUI_DRAW_CIRCLE_ID                  0x2008
#define GUI_DRAW_DISC_ID                    0x2009
#define GUI_DRAW_TRIANGLE_ID                0x200A
#define GUI_DRAW_GLYPH_ID                   0x200B
#define GUI_DRAW_BOX_ID                     0x200C
#define GUI_DRAW_RBOX_ID                    0x200D

#define GUI_DRAW_PROGRESS_BAR_ID            0x2100
#define GUI_DRAW_PROGRESS_BAR_TEXT_ID       0x2101
#define GUI_DRAW_SCROLLBAR_ID               0x2102
#define GUI_DRAW_SCROLLBAR_POS_ID           0x2103
#define GUI_DRAW_BUTTON_LEFT_ID             0x2104
#define GUI_DRAW_BUTTON_RIGHT_ID            0x2105
#define GUI_DRAW_BUTTON_CENTER_ID           0x2106
#define GUI_DRAW_MULTILINE_TEXT_ID          0x2107
#define GUI_DRAW_MULTILINE_TEXT_ALIGN_ID    0x2108
#define GUI_DRAW_MULTILINE_TEXT_FRAMED_ID   0x2109
#define GUI_DRAW_SLIGHTLY_ROUNDED_FRAME_ID  0x210A
#define GUI_DRAW_SLIGHTLY_ROUNDED_BOX_ID    0x210B
#define GUI_DRAW_BOLD_ROUNDED_FRAME_ID      0x210C
#define GUI_DRAW_BUBBLE_ID                  0x210D
#define GUI_DRAW_BUBBLE_STR_ID              0x210E
#define GUI_DRAW_STRING_FIT_WIDTH_ID        0x210F
#define GUI_DRAW_SCROLLABLE_TEXT_ID         0x2110
#define GUI_DRAW_SCROLLABLE_TEXT_CENTER_ID  0x2111

#define GUI_ICON_ADD_ID                     0x2200

#define HW_SPEAKER_PLAY_ID                  0x3000
#define HW_SPEAKER_STOP_ID                  0x3001
#define HW_SPEAKER_SET_VOLUME_ID            0x3002
#define HW_VIBRATOR_ON_ID                   0x3003
#define HW_VIBRATOR_OFF_ID                  0x3004
#define HW_LIGHT_SET_ID                     0x3005
#define HW_LIGHT_SEQUENCE_ID                0x3006
#define HW_LIGHT_BLINK_ON_ID                0x3007
#define HW_LIGHT_BLINK_OFF_ID               0x3008

typedef struct ProtocolPayload {
    uint16_t     id;
    uint32_t    data_size;
    uint8_t*    data;
} ProtocolPayload_t;

typedef struct ProtocolData {
    uint16_t    id;
    uint32_t    data_size;
    void*       data;
} ProtocolData_t;

typedef struct InputData {
    InputKey    key;
    InputType   type;
} InputData_t;

typedef struct GuiDrawData {
    uint16_t        draw_arr_size;
    ProtocolData_t* draw_arr;
} GuiDrawData_t;

typedef struct GuiDrawStrData {
    uint8_t     x;
    uint8_t     y;
    char*       str;
} GuiDrawStrData_t;

typedef struct GuiDrawStrAlignData {
    uint8_t     x;
    uint8_t     y;
    Align       horizontal;
    Align       vertical;
    char*       str;
} GuiDrawStrAlignData_t;

typedef struct GuiDrawFrameData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     width;
    uint8_t     height;
} GuiDrawFrameData_t;

typedef struct GuiDrawRFrameData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     width;
    uint8_t     height;
    uint8_t     radius;
} GuiDrawRFrameData_t;

typedef struct GuiDrawIconData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     icon_id;
} GuiDrawIconData_t;

typedef struct GuiDrawDotData {
    uint8_t     x;
    uint8_t     y;
} GuiDrawDotData_t;

typedef struct GuiDrawLineData {
    uint8_t     x1;
    uint8_t     y1;
    uint8_t     x2;
    uint8_t     y2;
} GuiDrawLineData_t;

typedef struct GuiDrawCircleData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     radius;
} GuiDrawCircleData_t;

typedef struct GuiDrawDiscData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     radius;
} GuiDrawDiscData_t;

typedef struct GuiDrawTriangleData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     base;
    uint8_t     height;
    CanvasDirection direction;
} GuiDrawTriangleData_t;

typedef struct GuiDrawGlyphData {
    uint8_t     x;
    uint8_t     y;
    uint16_t    glyph_char;
} GuiDrawGlyphData_t;

typedef struct GuiDrawBoxData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     width;
    uint8_t     height;
} GuiDrawBoxData_t;

typedef struct GuiDrawRBoxData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     width;
    uint8_t     height;
    uint8_t     radius;
} GuiDrawRBoxData_t;

typedef struct GuiIconAddData {
    uint8_t     icon_id;
    uint8_t     width;
    uint8_t     height;
    uint8_t*    frame_data;
} GuiIconAddData_t;

typedef struct SpeakerPlayData {
    float       frequency;
    float       volume;
} SpeakerPlayData_t;

typedef struct SpeakerSetVolumeData {
    float       volume;
} SpeakerSetVolumeData_t;

typedef struct LightSetData {
    Light       light;
    uint8_t     value;
} LightSetData_t;

typedef struct LightSequenceData {
    char*      sequence;
} LightSequenceData_t;

void protocol_payload_free(ProtocolPayload_t* payload);

void protocol_data_free(ProtocolData_t* data);

ProtocolData_t* protocol_receive(Cli* cli, uint32_t timeout_ms);

void protocol_send_empty(Cli* cli, uint16_t id);

void protocol_send(Cli* cli, uint16_t id, void* data);

void* protocol_decode(uint16_t id, uint32_t data_size, uint8_t* data);

ProtocolPayload_t* protocol_encode(uint16_t id, void* data);
