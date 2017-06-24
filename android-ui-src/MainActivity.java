package com.example.canvastry;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

/**
 * Created by rhatgadkar on 6/4/17.
 */

public class MainActivity extends AppCompatActivity {

    private static int VAR_SIZE = 21;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    /** Called when GUEST button is clicked */
    public void guestGame(View view) {
        Intent intent = new Intent(this, GameActivity.class);
        intent.putExtra("Username", "");
        intent.putExtra("Password", "");
        startActivity(intent);
    }

    public static boolean isValidCredential(String credential) {
        if (credential == null || credential.length() == 0)
            return false;
        if (credential.length() >= VAR_SIZE)
            return false;

        for (int i = 0; i < credential.length(); i++) {
            if (!Character.isDigit(credential.charAt(i)) &&
                    !Character.isLetter(credential.charAt(i))) {
                return false;
            }
        }
        return true;
    }

    /** Called when OK button is clicked */
    public void userGame(View view) {
        EditText userText = (EditText) findViewById(R.id.editText);
        EditText passText = (EditText) findViewById(R.id.editText2);

        boolean isValidUsername = isValidCredential(userText.getText().toString());
        boolean isValidPassword = isValidCredential(passText.getText().toString());
        if (!isValidUsername) {
            TextView errorText = (TextView) findViewById(R.id.textView);
            errorText.setText("Invalid formatted Username.");
        }
        if (!isValidPassword) {
            TextView errorText = (TextView) findViewById(R.id.textView);
            errorText.setText("Invalid formatted Password.");
        }
        if (isValidUsername && isValidPassword) {
            Intent intent = new Intent(this, GameActivity.class);
            intent.putExtra("Username", userText.getText().toString());
            intent.putExtra("Password", passText.getText().toString());
            startActivity(intent);
        }
    }
}
