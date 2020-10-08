use strict;
use warnings;
use Socket;

my($sock);

socket($sock, PF_INET, SOCK_STREAM, (getprotobyname("tcp"))[2]) or die "can't socket";
connect($sock, sockaddr_in(12342, inet_aton("127.0.0.1"))) or die "can't connect";

send($sock, "POST /cgi/echo_body.pl HTTP/1.1\r\nTRANSFER-encoding:chunked\r\n\r\n1;wow;this=is;funny=haha;2\r\na\r\n0\r\n", 0);

my $msg;
recv($sock, $msg, 2000, 0);

if ($msg != "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\na") {
    die "bad result";
}

close $sock or die "can't close";
