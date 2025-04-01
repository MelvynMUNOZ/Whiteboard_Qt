# Whiteboard_Qt

Interactive Whiteboard GUI and Server in Qt.


## FRONTEND

GUI to login with a username to server, specifying the server IP address.

Draw on the same canvas with other people.


## BACKEND

TCP Server on host machine, port 12345.

UDP Socket on host machine, port 12346.

To generate Backend Doxygen documentation, run the commands :
```
cd Backend
doxygen Doxyfile
```


## INTERNAL COMMUNICATION PROTOCOL

```
Big Endian

+-------------------------+----------------------------+
|  Backend                |  Frontend                  |
+-------------------------+----------------------------+

                        <---- Connect to TCP Server
            ACK_CONNECT ---->
            (TCP)

                        <---- REGISTER_CLIENT (TCP)
    ACK_REGISTER_CLIENT ---->
    (TCP)

                        <---- REGISTER_UDP_PORT (TCP)
  ACK_REGISTER_UDP_PORT ---->
  (TCP)

                        <---- REQUEST_ALL_CLIENT_INFOS (TCP)
           CLIENT_INFOS ---->
           (TCP)         * n

                        <---- DATA_CANVAS_CLIENT (UDP)
       DATA_CANVAS_SYNC ---->
       (UDP)             * n

    CLIENT_DISCONNECTED ---->
    (TCP)                * n
```


## AUTHORS

Melvyn MUNOZ, Raphael CAUSSE