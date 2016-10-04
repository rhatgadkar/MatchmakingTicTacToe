Networked matchmaking TicTacToe game.  Pairs of players automatically get matched to a game.

TODO:
- Implement locking for acceses to the map: ports_used.
- Clients should not determine whether the game is over or not based on the
  layout of the board.  When a client plays a winning move, the client should
  send a message to the server, and the server should forward this win to the
  other client.
- Support handling of SIGINT by the client.  When a client SIGINTs, this means
  that the client has "given up".  So then the other client wins the game.  If
  a SIGINT occurs when a client is waiting for another client to connect, then
  the corresponding child server process and port should be freed.
- Current sockets are using UDP.  Switch to TCP sockets.
