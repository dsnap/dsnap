#!/usr/bin/env perl
#By Kyle Pelton 2012


################################
#          EDIT THESE          #
################################
$image_name="old.im";
#how many threads to start
$cpu = "3";
#memory to allocate for vm in MB
$mem= "2048";
$driver = "e1000";
################################

$start_cmd= "qemu-system-i386  -redir tcp:2010::22 ";
$end_cmd= "-boot c  -hda $image_name -m $mem -smp $cpu -s -curses";
$net_cards = "";


if ($ARGV[0] < 1){
	print "./cards.pl <number of e1000>\n";
	exit(1);
}
for ($i=0; $i<$ARGV[0]; $i++){
	$net_cards = $net_cards ."-netdev user,id=dev$i -device $driver,netdev=dev$i ";
}
#create final command
$cmd = $start_cmd . $net_cards . $end_cmd;

print "running\n$cmd";
#Run the thang
system($cmd);
