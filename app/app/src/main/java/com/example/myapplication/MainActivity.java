package com.example.myapplication;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.example.myapplication.R;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class MainActivity extends AppCompatActivity {
    private TextView display;
    private EditText input;
    private EditText input2;
    private Button submit1;
    public Socket socket;
    private Button link;
    private TextView state;
    private Button off;
    private EditText temp;
    private Button test;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        state=(TextView)findViewById(R.id.state);  //状态框
        display = (TextView) findViewById(R.id.display); //温度显示框
        input = (EditText) findViewById(R.id.input);  //区间框1
        input2 = (EditText) findViewById(R.id.input2);  //区间框2
        submit1 = (Button) findViewById(R.id.submit);  //发送区间设定
        link = (Button) findViewById(R.id.link);        //连接
        off=(Button)findViewById(R.id.off); //关机按钮
        temp=(EditText) findViewById(R.id.temp);
        test=(Button)findViewById(R.id.test);

        //链接事件绑定
        link.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v){
                if (link.getText().equals("开启连接")){
                    //创建连接函数,1为创建链接,0为断开
                    linkOPT(1);
                    link.setText("断开连接");
                }
                else {
                    linkOPT(0);
                    link.setText("开启连接");
                }
            }
        });
        //区间设定事件绑定
        submit1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //获取数据
                String message = input.getText().toString();
                String message2 = input2.getText().toString();
                //只支持数字,小数
                if (message.matches("\\d+(\\.\\d+)?") && message2.matches("\\d+(\\.\\d+)?")) {
                    double num1 = Double.parseDouble(message);
                    double num2 = Double.parseDouble(message2);
                    //数据大小范围限制
                    if (num1 <= num2) {
                        if (num1<=99.99 && num1>=0 && num2<=99.99 && num2>=0)
                            if(link.getText()=="断开连接"){
                                message=String.valueOf((int) (num1*100));
                                message2=String.valueOf((int) (num2*100));
                                sendMessage("t "+message+','+message2);
                                input.setText("");
                                input2.setText("");
                            }
                            else{
                                Toast.makeText(MainActivity.this, "请开启连接", Toast.LENGTH_SHORT).show();
                            }
                        else {
                            Toast.makeText(MainActivity.this, "温度范围为0到99.99", Toast.LENGTH_SHORT).show();
                        }

                    } else {
                        Toast.makeText(MainActivity.this, "第一个数字必须小于第二个数字", Toast.LENGTH_SHORT).show();
                    }
                } else {
                    Toast.makeText(MainActivity.this, "只能输入数字或小数", Toast.LENGTH_SHORT).show();
                }
            }
        });
        //测试用
        test.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                double n=Double.parseDouble(temp.getText().toString());
                String m=String.valueOf((int)(n*100));
                sendMessage("q "+m);
            }
        });
        //关机事件绑定
        off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (off.getText().equals("开机")){
                    sendMessage("s");//单片机开机命令
                }
                else{
                    sendMessage("o");//关机命令
                }
            }
        });
    }
    //创建服务器连接线程
    private void linkOPT(final int opt){
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if (opt==1){
                        MainActivity.this.socket = new Socket("123.249.8.75", 8089);
                        recMesg();
                    }
                    else{
                        if( MainActivity.this.socket!=null)
                            MainActivity.this.socket.close();
                    }
                }
                catch (IOException e){
                    e.printStackTrace();
                }
            }
        }).start();
    }
    //接收信息线程
    private void recMesg(){
        new Thread(new Runnable(){
            @Override
            public void run(){
                try {
                    if (socket!=null) {
                        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                        while (true) {
                            final String message = in.readLine();
                            Log.d("tag",message);
                            if (message != null) {
                                if (message.equals("OVER")){
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            state.setText("加热完毕");
                                            off.setText("开机");
                                        }
                                    });
                                } else if (message.equals("RECO")) {
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            state.setText("正在加热");
                                            off.setText("关机");
                                        }
                                    });
                                } else  {
                                    //刷新温度显示
                                    runOnUiThread(new Runnable() {
                                        @Override
                                        public void run() {
                                            if (!message.isEmpty()) {
                                                String m = String.valueOf(Double.parseDouble(message) / 100);
                                                display.setText(m);
                                            }
                                        }
                                    });
                                }

                            } else {
                                break;
                            }
                        }
                    }
                }
                catch (IOException e){
                    e.printStackTrace();
                }
            }
        }).start();
    }
    //发送信息线程
    private void sendMessage(final String message) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    //Socket socket = new Socket("123.249.8.75", 8089);
                    PrintWriter out = new PrintWriter(MainActivity.this.socket.getOutputStream(), true);
                    //Log.d("TAG", message);
                    out.println(message);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }
}