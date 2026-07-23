using System;
using System.Net.Sockets;

namespace TSMS_Shutdown
{
    class Program
    {
        const int master_recv_udp_port = 8501;

        static void Main(string[] args)
        {
            Console.Write("Shutdown TSMS ...\n");

            try
            {
                var udpClient = new UdpClient();
                var data = new byte[2];
                data[0] = (byte)'X';       // id for commands from GUI
                data[1] = (byte)'z';       // <- command to force shutdown
                if (udpClient.Send(data, data.Length, "localhost", master_recv_udp_port) == data.Length)
                    Console.Write("UDP command was sent successfully.\n");
            }
            catch (Exception ex)
            {
                Console.Write("Error while creating UDP client!\n" + ex.Message);
            }

            System.Threading.Thread.Sleep(5000);
        }
    }
}
