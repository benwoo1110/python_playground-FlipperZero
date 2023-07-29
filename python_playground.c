#include "python_playground.h"

// ##############
// CLI
void cli_callback(Cli* cli, FuriString* args, void* ctx) {
    UNUSED(args);
    FURI_LOG_I(LOG_TAG, "CLI connection...");

    PPApp_t* app = ctx;
    if (app->cli_running) {
        return;
    }

    app->cli = cli;
    app->cli_running = true;
    view_dispatcher_switch_to_view(app->view_dispatcher, CONNECTED_VIEW_ID);    

    // connect
    protocol_send_empty(app->cli, CNT_FLIPPER_START_ID);

    while (app->cli_running) {
        if (!cli_is_connected(app->cli)) {
            app->cli_running = false;
            break;
        }

        // recieve payload
        ProtocolData_t* proto_data = protocol_receive(app->cli, 5);
        if (proto_data == NULL) {
            continue;
        }

        switch (proto_data->id) {
            case GUI_DRAW_ID: {
                ConnectedViewModel_t* vm = view_get_model(app->connected_view);
                if (vm->draw_data != NULL) {
                    for (uint16_t i = 0; i < vm->draw_data->draw_arr_size; i++) {
                        protocol_data_free(&vm->draw_data->draw_arr[i]);
                    }
                    free(vm->draw_data->draw_arr);
                    free(vm->draw_data);
                    vm->draw_data = NULL;
                }
                vm->draw_data = proto_data->data;
                view_commit_model(app->connected_view, true);
                break;
            }
            default: {
                FURI_LOG_E(LOG_TAG, "Unknown protocol id: %d", proto_data->id);
                break;
            }
        }

        free(proto_data);
    }

    // disconnect
    protocol_send_empty(app->cli, CNT_FLIPPER_STOP_ID);

    // cleanup
    app->cli = NULL;
    ConnectedViewModel_t* vm = view_get_model(app->connected_view);
    if (vm->draw_data != NULL) {
        for (uint16_t i = 0; i < vm->draw_data->draw_arr_size; i++) {
            protocol_data_free(&vm->draw_data->draw_arr[i]);
        }
        free(vm->draw_data->draw_arr);
        free(vm->draw_data);
        vm->draw_data = NULL;
    }
    view_commit_model(app->connected_view, true);

    // go back
    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
}

// ##############
// STATE
PPApp_t* state_init() {
    FURI_LOG_I(LOG_TAG, "Initing app...");
    PPApp_t* app = malloc(sizeof(PPApp_t));

    app->view_dispatcher    = view_dispatcher_alloc();
    app->gui                = furi_record_open(RECORD_GUI);
    app->mutex              = furi_mutex_alloc(FuriMutexTypeNormal);
    app->cli                = NULL;

    app->cli_running = false;

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_enable_queue(app->view_dispatcher);

    app->connected_view = connected_view_alloc(app);
    app->disconnected_view = disconnected_view_alloc(app);

    return app;
}

void state_free(PPApp_t* app) {
    FURI_LOG_I(LOG_TAG, "Freeing app...");
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);
    view_free(app->connected_view);
    view_free(app->disconnected_view);
    furi_mutex_free(app->mutex);
    free(app);
}

// ##############
// ENTRY
int32_t python_playground_app(void) {
    PPApp_t* app = state_init();
    
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_add_command(cli, "python_playground", CliCommandFlagParallelSafe, cli_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DISCONNECTED_VIEW_ID);
    view_dispatcher_run(app->view_dispatcher);

    cli_delete_command(cli, "python_playground");
    furi_record_close(RECORD_CLI);

    state_free(app);

    return 0;
}
