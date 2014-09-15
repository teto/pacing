WITH_GDB=1
#NS_LOG="$NS_LOG:TraceHelper:PointToPointHelper"
NS_LOG="OwdMainTest=*|prefix_all"
NS_LOG="$NS_LOG:Ipv4L3Protocol=warn|error"
NS_LOG="$NS_LOG:OwdClient"
NS_LOG="$NS_LOG:OWDServer"
#NS_LOG="$NS_LOG:Ipv4RoutingTable"
#NS_LOG="$NS_LOG:Ipv4StaticRouting"
#NS_LOG="$NS_LOG:UdpL4Protocol=*"
OUT="xp.txt"
#NS_LOG="$NS_LOG:MpTcpTestSuite=*|prefix_func:Socket=*"

export NS_LOG

if [ $WITH_GDB -gt 0 ]; then
	#COMMAND=
	echo 'gdb'
	read -r  command <<-'EOF'
		../../waf --run owd --command-template="gdb -ex run --args %s "
		EOF
else
	echo 'Without gdb'
	# you can add --out to redirect output to afile instead of standard output
	#--verbose 
	read -r  command <<-EOF
		../../waf --run owd
		EOF

fi



eval $command

echo "Exported:\n$NS_LOG"
echo "Executed Command:\n$command"

