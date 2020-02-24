package com.svetlichny.sunrise;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.io.IOException;
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
    private BluetoothAdapter bluetoothAdapter;
    private BluetoothSocket mmSocket;

    public interface Callback<R> {
        void onComplete(R result, String error);
    }

    public void connect(Callback<Void> callback) {
        final UUID SERVER_UUID = fromString("00001101-0000-1000-8000-00805F9B34FB");
        final String deviceName = "HC-06";

        executor.execute(() -> {
            bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            if (bluetoothAdapter == null) {
                handler.post(() -> callback.onComplete(null, "Device doesn't support Bluetooth"));
                return;
            }
            if (!bluetoothAdapter.isEnabled()) {
                bluetoothAdapter.enable();
            }

            // connect to first paired device with name = deviceName
            BluetoothDevice device = null;
            Set<BluetoothDevice> pairedDevices = bluetoothAdapter.getBondedDevices();
            for (BluetoothDevice d: pairedDevices) {
                if (d.getName().equals(deviceName)) {
                    device = d;
                    break;
                }
            }
            if (device == null) {
                handler.post(() -> callback.onComplete(null,
                        String.format("Bluetooth device with name %s not in the list of paired", deviceName)));
                return;
            }
            Log.d(TAG, "Device to connect = " + device.getName() + ' ' + device.getAddress());

            try {
                mmSocket = device.createRfcommSocketToServiceRecord(SERVER_UUID);
            } catch (IOException e) {
                handler.post(() -> callback.onComplete(null, "Socket's create() method failed: " + e.getMessage()));
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
                handler.post(() -> callback.onComplete(null, "Unable to connect: " + e.getMessage()));
                return;
            }

            handler.post(() -> {
                callback.onComplete(null, null);
            });
        });
    }

    public void send(String msg, Callback<Void> callback) {
        OutputStream mmOutStream;
        try {
            mmOutStream = mmSocket.getOutputStream();
        } catch (IOException e) {
            handler.post(() -> callback.onComplete(null, "Error creating output stream:  " + e.getMessage()));
            return;
        }
        byte[] bytes = msg.getBytes();
        try {
            mmOutStream.write(bytes);
        } catch (IOException e) {
            handler.post(() -> callback.onComplete(null, "Error sending data:  " + e.getMessage()));
            return;
        }

        handler.post(() -> callback.onComplete(null, null));

//            handler.post(() -> {
//                callback.onComplete(result);
//            });
    }

    public void disconnect(Callback<Void> callback) {
        try {
            mmSocket.close();
        } catch (IOException e) {
            handler.post(() -> callback.onComplete(null, "Could not close the client socket:  " + e.getMessage()));
            return;
        }
        handler.post(() -> callback.onComplete(null, null));
    }

}
