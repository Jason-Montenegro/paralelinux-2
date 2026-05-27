## Configuracion Switch L2

hostname Isla6                                                                  
!                                                                               
!                                                                               
no aaa new-model                                                                
system mtu routing 1500                                                         
ip subnet-zero                                                                  
ip dhcp excluded-address 172.16.123.97                                          
ip dhcp excluded-address 172.16.123.98                                          
!                                                                               
ip dhcp pool Isla6                                                              
   network 172.16.123.96 255.255.255.240                                        
   default-router 172.16.123.97                                                 
!                                                                               
!                                                                               
!                                                                               
!                                                                               
no file verify auto                                                             
spanning-tree mode pvst                                                         
spanning-tree extend system-id                                                  
!                                                                               
vlan internal allocation policy ascending                                       
!                                                                               
interface FastEthernet0/1                                                       
 switchport access vlan 260                                                     
 switchport mode access                                                         
!                                                                               
interface FastEthernet0/2                                                       
 switchport access vlan 260                                                     
 switchport mode access                                                         
!                                                                               
interface FastEthernet0/3                                                       
 switchport access vlan 260                                                     
 switchport mode access                                                          
!                                                                               
interface FastEthernet0/4
 switchport access vlan 260                                                     
 switchport mode access                                                       
!                                                                               
interface FastEthernet0/5
 switchport access vlan 260                                                     
 switchport mode access                                                        
!                                                                               
interface FastEthernet0/6 
 switchport access vlan 260                                                     
 switchport mode access                                                       
!                                                                               
interface FastEthernet0/7
 switchport access vlan 260                                                     
 switchport mode access                                                       
!                                                                               
interface FastEthernet0/8 
 switchport access vlan 260                                                     
 switchport mode access                                                       
!                                                                               
interface FastEthernet0/9  
 switchport access vlan 260                                                     
 switchport mode access                                                      
!                                                                               
interface FastEthernet0/10  
 switchport access vlan 260                                                     
 switchport mode access                                                     
!                                                                               
interface FastEthernet0/11 
 switchport access vlan 260                                                     
 switchport mode access                                                      
!                                                                               
interface FastEthernet0/12                                                      
!                                                                               
interface FastEthernet0/13                                                      
!                                                                               
interface FastEthernet0/14                                                      
!                                                                               
interface FastEthernet0/15                                                      
!                                                                               
interface FastEthernet0/16                                                      
!                                                                               
interface FastEthernet0/17                                                      
!                                                                               
interface FastEthernet0/18                                                      
!                                                                               
interface FastEthernet0/19                                                      
!                                                                               
interface FastEthernet0/20                                                      
!                                                                               
interface FastEthernet0/21                                                      
!                                                                               
interface FastEthernet0/22                                                      
!                                                                               
interface FastEthernet0/23                                                      
!                                                                               
interface FastEthernet0/24                                                      
!                                                                               
interface GigabitEthernet0/1  
 switchport mode trunk                                                  
!                                                                               
interface GigabitEthernet0/2                                                    
!                                                                               
interface Vlan1                                                                 
 no ip address                                                                  
 no ip route-cache                                                              
!                                                                               
interface Vlan260                                                               
 ip address 172.16.123.98 255.255.255.240                                       
 no ip route-cache                                                              
!                                                                               
ip http server                                                                  
!                                                                               
control-plane                                                                   
!                                                                               
!                                                                               
line con 0                                                                      
line vty 5 15                                                                   
!                                                                               
end

## Configuracion L3

hostname L3
!
enable secret 5 <secret>
enable password <contrasena>
!
no aaa new-model
ip subnet-zero
ip routing
!
!
!
!
no file verify auto
spanning-tree mode pvst
spanning-tree extend system-id
!         
vlan internal allocation policy ascending
!         
interface FastEthernet0/1
!         
interface FastEthernet0/2
!         
interface FastEthernet0/3
!         
interface FastEthernet0/4
!         
interface FastEthernet0/5
!         
interface FastEthernet0/6
!         
interface FastEthernet0/7
!         
interface FastEthernet0/8
!         
interface FastEthernet0/9
!         
interface FastEthernet0/10
!         
interface FastEthernet0/11
!         
interface FastEthernet0/12
!         
interface FastEthernet0/13
!         
interface FastEthernet0/14
!         
interface FastEthernet0/15
!         
interface FastEthernet0/16
!         
interface FastEthernet0/17
!         
interface FastEthernet0/18
!         
interface FastEthernet0/19
!         
interface FastEthernet0/20
!         
interface FastEthernet0/21
!         
interface FastEthernet0/22
!         
interface FastEthernet0/23
!         
interface FastEthernet0/24
!         
interface GigabitEthernet0/1
 switchport trunk encapsulation dot1q
 switchport mode trunk
!         
interface GigabitEthernet0/2
 switchport trunk encapsulation dot1q
 switchport mode trunk
!         
interface Vlan1
 no ip address
!         
interface Vlan230
 ip address 172.16.123.49 255.255.255.240
!
interface Vlan260
 ip address 172.16.123.97 255.255.255.240
!         
ip classless
ip http server
!         
!         
control-plane
!         
!         
line con 0
line vty 0 4
 password <contrasena>
 no login 
line vty 5 15
 password <contrasena>
 no login 
!         
!         
end

