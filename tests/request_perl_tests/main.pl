use strict;
use warnings;

my $dir = ".";

opendir(my $dh, $dir) or die "bad dir '$dir' because $!\n";

while ((my $filename = readdir($dh))) {
    if ((not ($filename eq "main.pl")) and (not ($filename eq ".")) and (not ($filename eq ".."))) {
        my $x = system("perl ./$filename");
        if ($x != 0) {
            print("[KO] $filename\n");
        } else {
            print("[OK] $filename\n");
        }
    }
}
closedir($dh) or die "bad close";
