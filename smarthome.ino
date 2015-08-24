#include <SPI.h>
#include <Ethernet.h>
#include <SoftwareSerial.h>

SoftwareSerial sSerial(3, 2); // RX,TX
//ip, mac etc
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02 };
IPAddress ip(192,168,1,8);
byte gateway[] = { 192, 168, 1, 1 };
byte subnet[] = { 255, 255, 255, 0 };
byte dnss[] = { 8, 8, 8, 8};
EthernetServer server(80);            


int state=0;     //by default light/board is OFF
int cstate=0;
int pkey=6;     //power key for GSM modem
char c = 0;     //for received HTTP header data

char detect;
int i=1;
IPAddress address;

void setup()
{
	digitalWrite(8, LOW);      //by default light is OFF, connected to pin 8
	pinMode(8, OUTPUT); 
	digitalWrite(9,LOW);      //electrical power board, pin 9
	pinMode(9,OUTPUT);
	pinMode(pkey, OUTPUT);   //power key of GSM modem

	sSerial.begin(9600);    //intialize serial for GSM modem & hw tx/rx
	Serial.begin(9600);
	Serial.println("Starting serial comm! Enter c to close led, o to light up LED, via serial!");  

	Serial.println("Getting IP address...");

	//static IP
	Ethernet.begin(mac, ip, gateway, subnet);

	Serial.print("IP address: ");

	address=Ethernet.localIP();
	Serial.println(address);

	server.begin(); 


	digitalWrite(pkey,LOW);
	delay(1000);
	digitalWrite(pkey,HIGH);
	delay(2000);
	digitalWrite(pkey,LOW);
	delay(3000);
	delay(2000);

	sSerial.print("AT+IPR=9600\r");
	delay(2000);
	sSerial.print("AT+CMGF=1\r");
	delay(500);
	sSerial.print("AT+CMGS=\"+1-your-cell-number\"\r");
	delay(500);
	sSerial.print("GSM Started.. ;)\r");
	delay(500);
	sSerial.write(0x1A);   //ctrl+z to send message
	delay(300);
	sSerial.print("AT+CNMI=2,2,0,0,0\r");   //Whenever a message arrives, modem should forward it to serial buffer
	delay(500);
	
}   //end setup


void loop()
{
	
	if ( sSerial.available() )    //checking for incoming messages and whether they match our pattern for controlling appliances
	{
		rf=sSerial.read();
		if(rf == '.')
		{
			rf=sSerial.read();
				if(rf == 'l')
				{
					rf=sSerial.read();
					if( rf == '0')            //light OFF
					{
					  state=0;
					  digitalWrite(8,LOW);
					  smsalert(0);
					}
					else if(rf == '1')    //light ON
					{
					 state=1;
					 digitalWrite(8,HIGH);
					 smsalert(1);
					}
				}
				else if ( rf == 'b')
				{
					rf=sSerial.read();
				    if(rf == '0')
					{
						cstate=0;
						digitalWrite(9,LOW);
						smsalert(3);        //board OFF
					}
					else if(rf=='1')
					{
						cstate=1;
						digitalWrite(9,HIGH);
						smsalert(2);       //board ON
					}
				}
		} //end if (rf=='.')
                  
	}
 	
	if( Serial.available() )     //to control light/LED using serial or GSM
	{
		char d = Serial.read();
		Serial.println(" Serial RX detect!");
		Serial.println(d); 
		if (d == 'o')
		{
			digitalWrite(8,HIGH);
			state=1;
		}
		else if (d == 'c')
		{
			digitalWrite(8,LOW);
			state=0;
		}
		else
		{
			state=state;
		}
	}

	EthernetClient client = server.available();
	boolean firstline = true;     //client's first HTTP request will be first line

	if (client) 
	{
		Serial.println("New Client");
		boolean blankline = true;     //flag for last line of http request

		while (client.connected()) 
		{

			if (client.available()) 
			{
				char c = client.read();

				if (c == '\n' && blankline)   //end of request from user.. so sending HTTP response
				{
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println();
					client.println("<html>");
					client.println("<head>"); 

					//Refresh to home page after 4 seconds
					client.print("<meta http-equiv=\"refresh\" content=\"4; URL=");
					client.print("http://");
					client.print(address);
					client.println("/\">");

					client.println("<title>Smart Home</title>");
					client.println("</head>");

					client.println("<center><p><h1>Web Based Remote Monitoring & Control!</h1></p><center><br/>");         
					client.print("<p><h2>Light status is <font color=red>");
					if(state)
					{
						client.println("ON :)");
					}
					else
					{
						client.println("OFFF :/");
					}
					client.println("</font></h2></p>");

					client.print("<p><h2>And board status is <font color=blue>");
					if(cstate)
					{
						client.println("ON :D");
					}
					else
					{
						client.println("OFF :(");
					}
					client.println("</font></h2></p>");

					//starting button
					client.println("<form  method=GET name=form>");

					outputbuttons(client);   //call function which will output buttons as HTML, based on state of light/LED

					client.println("</form><br />");
					client.println("</html>");
					break;    //done sending response/page.. now exit from WHILE n disconnect client
				}

				if (c == '\n')
				{
					firstline = false;     //if first line of header traveresed
					blankline = true;
				} 
				else if (c != '\r')      //if received normal character of http request
				{
					blankline = false;
				}


				//Detecting First HTTP request IFFFFF off/on button clicked. Contains GET /?light=0 or GET /?light=1
				if (firstline && c == '=') 
				{
					c = client.read();
					detect = c;

					if (detect == '0')   //turn OFF light button pressed!
					{
						digitalWrite(8, LOW);  //ligth's relay connection at pin 8.. 
						state=0;
						smsalert(0);
					}
					else if (detect == '1')   //turn ON LIGHT button pressed!
					{
						digitalWrite(8, HIGH);
						state=1;
						smsalert(1);
					}
					else if(detect == 'b')
					{
						digitalWrite(9,HIGH);   //turn ON board.. at pin 9.. by sending HIGH
						cstate=1;
						smsalert(2);
					}
					else if(detect == 'c')
					{
						digitalWrite(9,LOW);   //turn OFF board.. by sending HIGH
						cstate=0;
						smsalert(3);
					}
				}
			}   //IF after while end

		}     //while end

		// Response sent.
		delay(20);
		client.stop();
		
	}  //end if client

}   //end void loop


void outputbuttons(EthernetClient cc)
{
	if (state)
	{    
		cc.println("<button name=\"light\" value=\"0\" type=\"submit\" style=\"height:80px;width:200px\">Turn OFF light!</button>");
	}
	else
	{
		cc.println("<button name=\"light\" value=\"1\" type=\"submit\" style=\"height:80px;width:200px\">Turn ON light!</button>");
	}

	if(cstate)   //if currently board is on..
	{
		cc.println("<button name=\"board\" value=\"c\" type=\"submit\" style=\"height:80px;width:200px\">Turn OFF board!</button>");
	}
	else
	{
		cc.println("<button name=\"board\" value=\"b\" type=\"submit\" style=\"height:80px;width:200px\">Turn ON board..</button>");
	}

	cc.println("<p><center><h3>Much more can be added. Want to automate your home/office? Contact us!</h3></center></p>");
}



void smsalert(int btn)
{
	sSerial.print("AT+CMGF=1\r");
	delay(300);

	sSerial.print("AT+CMGS=\"+1-your-cell-number\"\r");
	delay(200);

	if(btn==0)   //light off button clicked
	{
		sSerial.print("ALERT: Light turned off!\r");
	}
	else if(btn == 1)
	{
		sSerial.print("ALERT: Light turned ON!\r");
	}
	else if(btn == 2)    //board ON
	{
		sSerial.print("B.ALERT: Board turned ON!\r");
	}
	else if (btn == 3)   //board OFF
	{
		sSerial.print("B.ALERT: Board turned OFF!\r");
	}

	delay(100);
	sSerial.write(0x1A);
	delay(100);
}
