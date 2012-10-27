iptables -A PREROUTING -t nat -i eth1 -p tcp --dport 80 -m iprange ! --dst-range 192.168.80.1-192.168.80.254 -j REDIRECT --to-port 3128
iptables -t nat -A POSTROUTING -o eth0 -s 192.168.80.0/24 -j MASQUERADE
