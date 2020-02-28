package com.svetlichny.sunrise;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

import static androidx.constraintlayout.widget.Constraints.TAG;
import static java.util.UUID.fromString;

public class BTLink {
    private final Executor executor = Executors.newSingleThreadExecutor();
    private final Handler handler = new Handler(Looper.getMainLooper());
    private final Callback callback;
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket mmSocket;
    private OutputStream mmOutputStream;
    private InputStream mmInputStream;
    Thread workerThread;
    byte[] readBuffer;
    int readBufferPosition;
    volatile boolean stopWorker;

    public interface Callback {
        void BtConnected();
        void BtDisconnected();
        void MsgSent();
        void MsgReceived(String msg);

        void onError(String error);
    }

    public BTLink(Callback callback) {
        this.callback = callback;
    }

    public void connect() {
        final UUID SERVER_UUID = fromString("00001101-0000-1000-8000-00805F9B34FB");
        final String deviceName = "HC-06";

        executor.execute(() -> {
            bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            if (bluetoothAdapter == null) {
                handler.post(() -> callback.onError("Device doesn't support Bluetooth"));
                return;
            }
            if (!bluetoothAdapter.isEnabled()) {
                bluetoothAdapter.enable();
            }

            // connect to first paired device with name = deviceName
            BluetoothDevice device = null;
            Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
            for (BluetoothDevice d : pairedDevices) {
                if (d.getName().equals(deviceName)) {
                    device = d;
                    break;
                }
            }
            if (device == null) {
                handler.post(() -> callback.onError(String.format("Bluetooth device with name %s not in the list of paired", deviceName)));
                return;
            }
            Log.d(TAG, "Device to connect = " + device.getName() + ' ' + device.getAddress());

            try {
                mmSocket = device.createRfcommSocketToServiceRecord(SERVER_UUID);
            } catch (IOException e) {
                handler.post(() -> callback.onError("Socket's create() method failed: " + e.getMessage()));
                return;
            }

            // Cancel discovery because it otherwise slows down the connection.
            bluetoothAdapter.cancelDiscovery();

            try {
                mmSocket.connect();
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException closeException) {
                    Log.e(TAG, "Could not close the client socket", closeException);
                }
                handler.post(() -> callback.onError("Unable to connect: " + e.getMessage()));
                return;
            }

            // open streams
            try {
                mmOutputStream = mmSocket.getOutputStream();
            } catch (IOException e) {
                handler.post(() -> callback.onError("Error creating output stream:  " + e.getMessage()));
                return;
            }
            try {
                mmInputStream = mmSocket.getInputStream();
            } catch (IOException e) {
                handler.post(() -> callback.onError("Error creating input stream:  " + e.getMessage()));
                return;
            }

            beginListenForData();
            handler.post(() -> {
                callback.BtConnected();
            });
        });
    }

    private void beginListenForData() {
        final byte delimiter = '\n';

        stopWorker = false;
        workerThread = new Thread(new Runnable() {
            public void run() {
                while (!Thread.currentThread().isInterrupted() && !stopWorker) {
                    try {
                        int bytesAvailable = mmInputStream.available();
                        if (bytesAvailable > 0) {
                            byte[] packetBytes = new byte[bytesAvailable];
                            int bytes = mmInputStream.read(packetBytes);
                            String msg = new String(packetBytes, 0, bytes);
                            handler.post(() -> callback.MsgReceived(msg));
                        }
                    } catch (IOException ex) {
                        stopWorker = true;
                    }
                }
            }
        });

        workerThread.start();
    }

    public void send(String msg) {
        byte[] bytes = msg.getBytes();
        try {
            if (mmOutputStream == null) {
                handler.post(() -> callback.onError("Not connected"));
                return;
            }
            mmOutputStream.write(bytes);
        } catch (IOException e) {
            handler.post(() -> callback.onError("Error sending data:  " + e.getMessage()));
            return;
        }

        handler.post(() -> callback.MsgSent());
    }


    public void disconnect() {
        stopWorker = true;
        try {
            mmOutputStream.close();
        } catch (IOException e) {
            handler.post(() -> callback.onError("Could not close the output stream:  " + e.getMessage()));
            return;
        }
        try {
            mmInputStream.close();
        } catch (IOException e) {
            handler.post(() -> callback.onError("Could not close the intput stream:  " + e.getMessage()));
            return;
        }
        try {
            mmSocket.close();
        } catch (IOException e) {
            handler.post(() -> callback.onError("Could not close the client socket:  " + e.getMessage()));
            return;
        }
        handler.post(() -> callback.BtDisconnected());
    }
}
