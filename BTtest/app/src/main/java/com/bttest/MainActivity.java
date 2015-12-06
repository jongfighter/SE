package com.bttest;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final String TAG = "bluetooth2";

    private ImageButton btnSet;
    private ImageButton btnPin;
    private ImageButton btnPinset;
    Handler h;      //handler for receiving message from the arduino
    private int limitTime = 50;

    final int RECEIVE_MESSAGE = 1;        // Status  for Handler
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private StringBuilder sb = new StringBuilder();

    private ConnectedThread mConnectedThread;

    // SPP UUID service
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // MAC-address of Bluetooth module (you must edit this line)
    private static String address = "98:D3:31:70:25:F8";            //MAC address of bluetooth module

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        btnSet = (ImageButton) findViewById(R.id.setButton);                  // button for time setting
        btnSet.setEnabled(false);
        btnPinset = (ImageButton) findViewById(R.id.pinsetButton);                  // button for PIN-CODE setting
        btnPinset.setEnabled(false);
        btnPin = (ImageButton) findViewById(R.id.pinButton);

        h = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECEIVE_MESSAGE:                                                   // if receive massage
                        byte[] readBuf = (byte[]) msg.obj;
                        String strIncom = new String(readBuf, 0, msg.arg1);                 // create string from bytes array
                        sb.append(strIncom);                                                // append string
                        int endOfLineIndex = sb.indexOf("\r\n");                            // determine the end-of-line
                        if (endOfLineIndex > 0) {                                            // if end-of-line,
                            String sbprint = sb.substring(0, endOfLineIndex);               // extract string
                            sb.delete(0, sb.length());                                      // and clear
                            if (sbprint.equals("LONG")) {
                                getUp();                           //if the arduino said LONG, then wakeup()
                            }
                            else if (sbprint.equals("DENY")){
                                dialogWrong();
                            }
                            else if (sbprint.length() > 4 && sbprint.substring(0, 5).equals("LOGIN")){
                                btnSet.setEnabled(true);
                                btnPinset.setEnabled(true);
                                btnSet.setAlpha(0.8f);
                                btnPinset.setAlpha(0.8f);
                                limitTime = Integer.parseInt(sbprint.substring(5));
                            }
                        }
                        break;
                }
            };
        };

        btAdapter = BluetoothAdapter.getDefaultAdapter();       // get Bluetooth adapter
        checkBTState();         //check bluetooth on

        btnSet.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                dialogSetting();
            }
        });

        btnPinset.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                dialogPINSetting();
            }
        });

        btnPin.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                dialogPin();
            }
        });
    }
/*
    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        if(Build.VERSION.SDK_INT >= 10){
            try {
                final Method  m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
                return (BluetoothSocket) m.invoke(device, MY_UUID);
            } catch (Exception e) {
                Log.e(TAG, "Could not create Insecure RFComm Connection",e);
            }
        }
        return  device.createRfcommSocketToServiceRecord(MY_UUID);
    }
*/
    @Override
    public void onResume() {
        super.onResume();

        // Set up a pointer to the remote node using it's address.
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        // Two things are needed to make a connection:
        //   A MAC address, which we got above.
        //   A Service ID or UUID.  In this case we are using the
        //     UUID for SPP.

        try {
            btSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        } catch (IOException e) {
            errorExit("Fatal Error", "In onResume() and socket create failed: " + e.getMessage() + ".");
        }

        // Discovery is resource intensive.  Make sure it isn't going on
        // when you attempt to connect and pass your message.
        btAdapter.cancelDiscovery();

        // Establish the connection.  This will block until it connects.
        try {
            btSocket.connect();
        } catch (IOException e) {
            try {
                btSocket.close();
            } catch (IOException e2) {
                errorExit("Fatal Error", "In onResume() and unable to close socket during connection failure" + e2.getMessage() + ".");
            }
        }

        // Create a data stream so we can talk to server.

        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();
    }

    @Override
    public void onPause() {
        super.onPause();

        try     {
            btSocket.close();
        } catch (IOException e2) {
            errorExit("Fatal Error", "In onPause() and failed to close socket." + e2.getMessage() + ".");
        }
    }

    private void checkBTState() {
        // Check for Bluetooth support and then check to make sure it is turned on
        // Emulator doesn't support Bluetooth and will return null
        if(btAdapter==null) {
            errorExit("Fatal Error", "Bluetooth not support");
        } else {
            if (btAdapter.isEnabled()) {
                Log.d(TAG, "...Bluetooth ON...");
            } else {
                //Prompt user to turn on Bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }

    private void errorExit(String title, String message){
        Toast.makeText(getBaseContext(), title + " - " + message, Toast.LENGTH_LONG).show();
        finish();
    }

    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[256];  // buffer store for the stream
            int bytes; // bytes returned from read()

            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);        // Get number of bytes and message in "buffer"
                    h.obtainMessage(RECEIVE_MESSAGE, bytes, -1, buffer).sendToTarget();     // Send to message queue Handler
                } catch (IOException e) {
                    break;
                }
            }
        }

        /* Call this from the main activity to send data to the remote device */
        public void write(String message) {
            Log.d(TAG, "...Data to send: " + message + "...");
            byte[] msgBuffer = message.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                Log.d(TAG, "...Error data send: " + e.getMessage() + "...");
            }
        }
    }

    public void getUp() {
        //when time passed too long, show dialog
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final LayoutInflater inflater =
                (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);
        final View Viewlayout = inflater.inflate(R.layout.dialog_wakeup,
                (ViewGroup) findViewById(R.id.layout_wakeup_dialog));

        builder.setTitle("Get Up").setView(Viewlayout)
                .setPositiveButton("OK", null)
                .create()
                .show();
    }


    public void dialogPin() {
        //Pincode is needed to change time option
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final LayoutInflater inflater =
                (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);
        final View Viewlayout = inflater.inflate(R.layout.dialog_pin,
                (ViewGroup) findViewById(R.id.layout_pin_dialog));

        final EditText pinInput = (EditText) Viewlayout.findViewById(R.id.pin);

        builder.setTitle("Enter PIN-Code").setView(Viewlayout)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                mConnectedThread.write("SIGN" + pinInput.getText().toString());
            }
        })
                .setNegativeButton("Cancel", null)
                .create()
                .show();
    }

    public void dialogPINSetting() {
        //send New PIN code
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final LayoutInflater inflater =
                (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);
        final View Viewlayout = inflater.inflate(R.layout.dialog_pinsetting,
                (ViewGroup) findViewById(R.id.layout_PINsetting_dialog));

        final EditText pin1 = (EditText) Viewlayout.findViewById(R.id.pin1);
        final EditText pin2 = (EditText) Viewlayout.findViewById(R.id.pin2);

        builder.setTitle("PIN-Code Setting").setView(Viewlayout)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                        public void onClick(DialogInterface dialog, int which) {
                        if(pin1.getText().toString().length() != 4)     //Too short
                            dialogWrong();
                        if(pin1.getText().toString().equals(pin2.getText().toString()))     //good case
                            mConnectedThread.write("PIN" + pin1.getText().toString());
                        else
                            dialogWrong();      //confirm unmatched
                    }
                })
                .setNegativeButton("Cancel", null)
                .create()
                .show();
    }

    public void dialogSetting() {
        //send time information
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final LayoutInflater inflater =
                (LayoutInflater) this.getSystemService(LAYOUT_INFLATER_SERVICE);
        final View Viewlayout = inflater.inflate(R.layout.dialog_setting,
                (ViewGroup) findViewById(R.id.layout_setting_dialog));

        final EditText time = (EditText) Viewlayout.findViewById(R.id.time);

        time.setText("" + limitTime);

        builder.setTitle("Time Setting").setView(Viewlayout)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        mConnectedThread.write("SET" + time.getText().toString());      //ex: SET10
                    }
                })
                .setNegativeButton("Cancel", null)
                .create()
                .show();
    }


    public void dialogWrong() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Pin-Code Error")
                .setMessage("Enter the right Pin-Code")
                .setCancelable(true)
                .setPositiveButton("OK", null)
                .create()
                .show();
    }
    /* for testing
    @Override
    public void onBackPressed() {
        btnSet.setEnabled(true);
        btnPinset.setEnabled(true);
    }
    */
}