chmod +x ./ubuntu_tester
chmod +x ./youpi_tester/ubuntu_cgi_tester

webserv -c ./youpi_tester/config.json &
sleep 1;
./ubuntu_tester http://localhost:1234