#include "protocol.h"

void protocol_payload_free(ProtocolPayload_t* payload) {
    if (payload == NULL) {
        return;
    }
    if (payload->data != NULL) {
        free(payload->data);
    }
    free(payload);
}

void protocol_data_free(ProtocolData_t* data) {
    if (data == NULL) {
        return;
    }
    if (data->data != NULL) {
        free(data->data);
    }
    free(data);
}

ProtocolData_t* protocol_recieve(Cli* cli, uint32_t timeout_ms) {
    ProtocolPayload_t* payload = malloc(sizeof(ProtocolPayload_t));
    if (cli_read_timeout(cli, (uint8_t*)&payload->id, sizeof(payload->id), timeout_ms) == 0) {
        protocol_payload_free(payload);
        return NULL;
    }
    if (cli_read_timeout(cli, (uint8_t*)&payload->data_size, sizeof(payload->data_size), timeout_ms) == 0) {
        protocol_payload_free(payload);
        return NULL;
    }
    payload->data = malloc(payload->data_size);
    if (cli_read_timeout(cli, payload->data, payload->data_size, timeout_ms) == 0) {
        protocol_payload_free(payload);
        return NULL;
    }
    ProtocolData_t* data = protocol_decode(payload);
    protocol_payload_free(payload);
    return data;
}

void protocol_send(Cli* cli, int16_t id, void* data) {
    ProtocolPayload_t* payload = protocol_encode(id, data);
    if (payload == NULL) {
        return;
    }
    cli_write(cli, (uint8_t*)&payload->id, sizeof(payload->id));
    cli_write(cli, (uint8_t*)&payload->data_size, sizeof(payload->data_size));
    cli_write(cli, payload->data, payload->data_size);
    protocol_payload_free(payload);
}

ProtocolData_t* gui_draw_str_decode(ProtocolPayload_t* payload) {
    GuiDrawStrData_t* data = malloc(sizeof(GuiDrawStrData_t));
    data->x = payload->data[0];
    data->y = payload->data[1];
    data->str = malloc(payload->data_size - 2);
    memcpy(data->str, payload->data + 2, payload->data_size - 2);
    ProtocolData_t* result = malloc(sizeof(ProtocolData_t));
    result->id = payload->id;
    result->data = data;
    return result;
}

ProtocolData_t* gui_draw_frame_decode(ProtocolPayload_t* payload) {
    GuiDrawFrameData_t* data = malloc(sizeof(GuiDrawFrameData_t));
    data->x = payload->data[0];
    data->y = payload->data[1];
    data->width = payload->data[2];
    data->height = payload->data[3];
    ProtocolData_t* result = malloc(sizeof(ProtocolData_t));
    result->id = payload->id;
    result->data = data;
    return result;
}

ProtocolData_t* protocol_decode(ProtocolPayload_t* payload) {
    switch (payload->id) {
        case GUI_DRAW_STR_ID: return gui_draw_str_decode(payload);
        case GUI_DRAW_FRAME_ID: return gui_draw_frame_decode(payload);
        default: return NULL;
    }
}

ProtocolPayload_t* input_encode(int16_t id, void* data) {
    InputData_t* input_data = data;
    ProtocolPayload_t* payload = malloc(sizeof(ProtocolPayload_t));
    payload->id = id;
    payload->data_size = 2;
    payload->data = malloc(payload->data_size);
    payload->data[0] = input_data->key;
    payload->data[1] = input_data->type;
    return payload;
}

ProtocolPayload_t* protocol_encode(int16_t id, void* data) {
    switch (id) {
        case INPUT_ID: return input_encode(id, data);
        default: return NULL;
    }
}
