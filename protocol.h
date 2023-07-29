#include <stdlib.h>

#include <furi.h>
#include <cli/cli.h>
#include <input/input.h>

#define TEST_ID                 0x0001

#define INPUT_ID                0x1001

#define GUI_DRAW_ID             0x2000
#define GUI_DRAW_STR_ID         0x2001
#define GUI_DRAW_STR_ALIGN_ID   0x2002
#define GUI_DRAW_FRAME_ID       0x2003

typedef struct ProtocolPayload {
    uint16_t    id;
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

typedef struct GuiDrawFrameData {
    uint8_t     x;
    uint8_t     y;
    uint8_t     width;
    uint8_t     height;
} GuiDrawFrameData_t;

void protocol_payload_free(ProtocolPayload_t* payload);

void protocol_data_free(ProtocolData_t* data);

ProtocolData_t* protocol_receive(Cli* cli, uint32_t timeout_ms);

void protocol_send(Cli* cli, uint16_t id, void* data);

void* protocol_decode(uint16_t id, uint32_t data_size, uint8_t* data);

ProtocolPayload_t* protocol_encode(uint16_t id, void* data);
