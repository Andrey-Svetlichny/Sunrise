package com.svetlichny.sunrise;

import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;
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

public class MainActivity extends AppCompatActivity implements BTLink.Callback {
    BTLink bTLink;
    TextView logView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        ToggleButton toggle = findViewById(R.id.toggleButton);
        toggle.setOnCheckedChangeListener((buttonView, isChecked) -> {
            String msg = isChecked ? "1\n" : "0\n";
            bTLink.send(msg);
        });

        bTLink = new BTLink(this);

        logView = findViewById(R.id.logTextView);
    }

    @Override
    protected void onPause(){
        super.onPause();
        bTLink.disconnect();
    }

    @Override
    protected void onResume(){
        super.onResume();
        bTLink.connect();
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

    @Override
    public void BtConnected() {
        Log.d(TAG, "BTLink connected.");
        String currentTime = new SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(new Date());
        String sunriseTime = "06:30";
        String command = "S" + currentTime + "|" + sunriseTime + "\n";
        bTLink.send(command);
    }

    @Override
    public void BtDisconnected() {
        Log.d(TAG, "BTLink disconnected");
    }

    @Override
    public void MsgSent() {
        Log.d(TAG, "Message sent.");
    }

    @Override
    public void MsgReceived(String msg) {
        Log.d(TAG, "Message received = " + msg);
        logView.append(msg + "\n");
    }

    @Override
    public void onError(String error) {
        Log.e(TAG, "BTLink error: " + error);
    }
}
