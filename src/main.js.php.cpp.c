#define RAYGUI_IMPLEMENTATION
#include <enet.h>
#include <raygui.h>
#include <raylib.h>
#include <stdio.h>

typedef unsigned ConnectionType;
enum ConnectionTypeBits {
    CONNECTION_TYPE_NONE   = 0,
    CONNECTION_TYPE_CLIENT = 1,
    CONNECTION_TYPE_SERVER = 2
};

ConnectionType selectConnectionType ();
void clientLoop ();
void serverLoop ();

int main (int argc, char** argv) {

    if (enet_initialize())
    {
        return EXIT_FAILURE;
    }

    InitWindow (800, 600, "Simple multipayer game");

    switch (selectConnectionType ()) {

    case CONNECTION_TYPE_CLIENT:
        clientLoop ();
        break;
    case CONNECTION_TYPE_SERVER:
        serverLoop ();
        break;
    default:
        break;
    }

    CloseWindow ();

    enet_deinitialize();
}

ConnectionType selectConnectionType () {

    ConnectionType connectionType = CONNECTION_TYPE_NONE;

    while (!WindowShouldClose () && !connectionType) {

        BeginDrawing (); {

            ClearBackground (BLACK);
            DrawFPS(10, 10);
            DrawText("Select client or server [C/S]", GetRenderWidth() * 0.30, GetRenderHeight() >> 1, 20, PINK);

            if (IsKeyPressed (KEY_C)) {

                connectionType = CONNECTION_TYPE_CLIENT;
            }
            else if (IsKeyPressed (KEY_S)) {

                connectionType = CONNECTION_TYPE_SERVER;
            }
        }
        EndDrawing ();
    }
    return connectionType;
}

void clientLoop () {

    ENetAddress address;
    ENetHost* client;
    ENetEvent event;
    ENetPeer* peer;
    ENetPacket* packet;
    char message[512] = {};

    client = enet_host_create (NULL, 1, 2, 0, 0);

    if (client) {
        puts("wiadomo kto wiadomo co client istnieje koleszko ciesz sie i baw");
    }

    enet_address_set_host (&address, "127.0.0.1");
    address.port = 3107;

    peer = enet_host_connect (client, &address, 1, 0);

    if (peer) {
        puts("wiadomo kto wiadomo co peer istnieje koleszko ciesz sie i baw peer peer peer peer perr eppepperer");
    }
    
    if (!peer) {
        fprintf (stderr, "No avaible peers for initiating an ENet bro.");
        exit (EXIT_FAILURE);
    }
    bool connected = 0;

    Rectangle textBoxBounds = { 50, 200, 200, 40 };
    char textBoxText[512] = "\0";
    Rectangle sendButtonBounds = { 50, 300, 200, 40 };

        while (!WindowShouldClose()) {

            while (enet_host_service(client, &event, 0) > 0) {

                switch (event.type) {

                    case ENET_EVENT_TYPE_CONNECT: {
                        printf("Connected to server wiadomo co wiadomo kto wiadomo kogo\n");
                        connected = 1;
                    } break;

                    case ENET_EVENT_TYPE_RECEIVE: {
                        printf("Message recieved! %s\n od wiadomo kogo wiadmoco co wiadomo czemu", event.packet->data);
                        strcpy(message, event.packet->data);
                        enet_packet_destroy(event.packet);
                    } break;

                    case ENET_EVENT_TYPE_DISCONNECT:
                    case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                        printf("A client has disconnected\n");
                        break;
                }
            }
        
        if (IsKeyPressed (KEY_S) && IsKeyPressed (KEY_LEFT_SHIFT) && connected) {
            if (peer->state == ENET_PEER_STATE_CONNECTED) {
                packet = enet_packet_create ("wyslalem", sizeof("wyslalem"), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send (peer, 0, packet);
                enet_packet_destroy (packet);
            }

        }

        BeginDrawing (); {
            
            ClearBackground (PINK);
        
            DrawText("Clint", GetScreenWidth() / 1.075, GetScreenHeight() / 50, 20, BLACK);

            GuiSetStyle (DEFAULT, TEXT_SIZE, 20);

            textBoxText[GuiTextBox(textBoxBounds, textBoxText, 64,1)];

            if (GuiButton ((Rectangle) { 50, 400, 200, 40 }, "Disconnect") && connected) {
                enet_peer_disconnect (peer, 0);
                connected = 0;
                TraceLog (LOG_INFO, "Disconnected with server");
                strcpy(message, "Rozlaczono");
            }

            if (GuiButton ((Rectangle) { 50, 500, 200, 40 }, "Connect") && !connected) {
                peer = enet_host_connect (client, &address, 0, 0);
                if (!peer) {
                    TraceLog (LOG_ERROR, "Failed to connect to the server");
                    strcpy (message, "Blad z polaczeniem z serwerem");
                }
                strcpy (message, "Polaczono z serwerem");
                connected = 1;
            }

            if (GuiButton (sendButtonBounds, "Send")) {
                if (!strcmp(textBoxText, "") && !connected && peer->state != ENET_PEER_STATE_CONNECTED) {
                    TraceLog (LOG_ERROR, "Puste jest kurwa");
                }
                else {
                    packet = enet_packet_create(textBoxText, strlen(textBoxText), ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send (peer, 0, packet);
                    textBoxText[0] = '\0';
                    enet_packet_destroy (packet);
                }
            }

            DrawText (message, GetRenderWidth () >> 1, (GetRenderHeight () >> 1) - 30, 20, BLACK);
            DrawFPS (10, 10);
        }

        EndDrawing ();
    }
    enet_host_destroy (client);
}

void serverLoop () {

    ENetAddress address;
    ENetHost* server;
    ENetEvent event;
    ENetPacket* packet;
    address.host = ENET_HOST_ANY;
    address.port = 3107;

    if (!(server = enet_host_create(&address, 32, 2, 0, 0)))
    {
        fprintf(stderr, "Failed to create a server");
        exit(EXIT_FAILURE);
    }
    char message[512] = {};

    while (!WindowShouldClose ()) {

        while (enet_host_service(server, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                printf ("A new client connected from %x:%u.\n", 
                        event.peer -> address.host,
                        event.peer -> address.port);
                event.peer -> data = "Client information";
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
                        event.packet -> dataLength,
                        event.packet -> data,
                        event.peer -> data,
                        event.channelID);
                strcpy(message, event.packet->data);
                packet = enet_packet_create ("Dostalem", strlen("Dostalem") + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send (event.peer, 0, packet);
                enet_packet_destroy (event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf ("%s disconnected.\n", event.peer->data);
                event.peer->data = NULL;
                break;
            default:
                break;
            }
        }

        BeginDrawing (); {

            ClearBackground (BLUE);
            DrawText ("Sever", GetRenderWidth () >> 1, GetRenderHeight () >> 1, 20, BLACK);
            DrawText (message, GetRenderWidth () >> 1, (GetRenderHeight () >> 1) - 30, 20, BLACK);
            DrawFPS (10, 10);
        }

        EndDrawing ();
    }

    enet_host_destroy(server);
}
