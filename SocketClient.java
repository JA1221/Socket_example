import java.net.InetSocketAddress;
import java.net.Socket;
import java.io.BufferedOutputStream;

public class SocketClient {
    private String address = "127.0.0.1";// 連線的ip
    private int port = 4567;// 連線的port
    
    public SocketClient() {
        Socket client = new Socket();
        InetSocketAddress isa = new InetSocketAddress(this.address, this.port);
        try {
            client.connect(isa, 10000);
            BufferedOutputStream out = new BufferedOutputStream(client
                    .getOutputStream());
            // 送出字串
            out.write("Send From JAVA_Client ".getBytes());
            out.flush();
            out.close();
            out = null;
            client.close();
            client = null;
        } catch (java.io.IOException e) {
            System.out.println("Socket連線有問題 !");
            System.out.println("IOException :" + e.toString());
        }
    }
    public static void main(String args[]) {
        new SocketClient();
    }
}