package com.svetlichny.sunrise;

import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.ToggleButton;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.floatingactionbutton.FloatingActionButton;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import static androidx.constraintlayout.widget.Constraints.TAG;


// Commands
// 1 - switch ON
// 0 - switch OFF
// S<CurrentTime>|<SunriseTime> - sync time and set sunrise time

public class MainActivity extends AppCompatActivity {
    BTLink bTLink;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                connectBluetoothAndSendMessage("0");
            }
        });

        ToggleButton toggle = (ToggleButton) findViewById(R.id.toggleButton);
        toggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                String msg = isChecked ? "1\n" : "0\n";
                bTLink.send(msg, (__, sendError) -> {
                    if (sendError != null){
                        Log.e(TAG, "Error: " + sendError);
                    } else {
                        Log.d(TAG, "Message sent.");
                    }
                });
            }
        });

        bTLink = new BTLink();
    }

    @Override
    protected void onPause(){
        super.onPause();
        bTLink.disconnect((__, error) -> {
            if (error != null){
                Log.e(TAG, "Error: " + error);
            }
        });
    }

    @Override
    protected void onResume(){
        super.onResume();
        bTLink.connect((__, error) -> {
            if (error != null){
                Log.e(TAG, "Error: " + error);
            }
            else {
                Log.d(TAG, "BTLink connected.");
                String currentTime = new SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(new Date());
                String sunriseTime = "07:00:00";
                String command = "S" + currentTime + "|" + sunriseTime + "\n";
                bTLink.send(command, (___, sendError) -> {
                    if (sendError != null){
                        Log.e(TAG, "Error: " + sendError);
                    } else {
                        Log.d(TAG, "Message sent.");
                    }
                });
            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void connectBluetoothAndSendMessage(String msg) {
        BTLink.Callback<Void> callback = (__, error) -> {
            if (error != null){
                Log.e(TAG, "Error: " + error);
            }
            else {
                Log.d(TAG, "BTLink connected.");
                bTLink.send(msg, (___, sendError) -> {
                    if (sendError != null){
                        Log.e(TAG, "Error: " + sendError);
                    } else {
                        Log.d(TAG, "Message sent.");
                        bTLink.disconnect((____, disconnectError) -> {});
                    }
                });
            }
        };
        bTLink.connect(callback);
    }

}
