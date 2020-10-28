use strict;
use warnings;
use Socket;

my($sock);

socket($sock, PF_INET, SOCK_STREAM, (getprotobyname("tcp"))[2]) or die "can't socket";
connect($sock, sockaddr_in(4242, inet_aton("127.0.0.1"))) or die "can't connect";

send($sock, "POST /cgi/tc_echo.clua HTTP/1.1\r\nhost: localhost:4242\r\nTRANSFER-encoding:chunked\r\n\r\n", 0);

for (my $ctr = 0; $ctr <= 1000; $ctr += 1) {
    send($sock, "a\r\n0123456789\r\n", 0);
}

send($sock, "0\r\n\r\n", 0);

my $full = "";
my $msg;
my $total_read_shouldbe = 15073;
my $total_read = 0;
while ($total_read < $total_read_shouldbe) {
    print("total_read $total_read\n");
    recv($sock, $msg, 4096, 0);
    $total_read += length($msg);
    $full .= $msg;
}

print($full);

close $sock or die "can't close";
