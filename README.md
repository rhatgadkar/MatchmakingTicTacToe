Networked matchmaking TicTacToe game.  Pairs of players automatically get matched to a game.  The server is hosted on an AWS EC2.

Available for download on Google Play:  https://play.google.com/store/apps/details?id=com.mmttt.canvastry&hl=en-US

![image](https://user-images.githubusercontent.com/8902454/32877499-ae23c66c-ca56-11e7-9fef-c860b496472d.png)

Clients connect to the parent server in port 4950. The matches are held in up to 100 other child
servers in ports 4951 – 5050.

Clients get assigned to one of the child server ports. The priority is to choose child servers
which already have 1 client in them. A queue is used to determine what child servers incoming
clients should join.

Timeouts are used to prevent stalled games and wasted resources. If a second client does not
join a child server within 15 seconds, the child server closes, and the initial client will
automatically begin searching for a new child server to join.

Players have up to 15 seconds to play a move. If they can't play a move in that time, they are
considered to have given up.

Connection losses are considered. If a child server is full (contains both players) and a move
has not been received within 40 seconds, this is considered as a connection loss and the child
server closes. No player wins in this case.

If a player enters username/password credentials, their win/loss records get saved in a database.
