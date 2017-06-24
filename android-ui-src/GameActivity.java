package com.example.canvastry;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.view.View;

public class GameActivity extends AppCompatActivity implements ITicTacToe {


    private Button _quitbutton;

    private String _username;
    private String _password;

    private TextView _winrecordfield;
    private TextView _lossrecordfield;
    private TextView _opponentnamefield;
    private TextView _numpplfield;
    private TextView _turnfield;
    private TextView _playerfield;
    private TextView _timerfield;

    private MyCanvasView _display;

    private Board _board;
    private Client _c;
    private Game _game;
    private ITicTacToe _ttt;

    private String _gameOverMsg;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        _ttt = this;

        Intent intent = getIntent();
        _username = intent.getStringExtra("Username");
        _password = intent.getStringExtra("Password");

        setContentView(R.layout.activity_game);

        _display = (MyCanvasView) findViewById(R.id.displayView);

        _turnfield = (TextView) findViewById(R.id.turnField);
        _playerfield = (TextView) findViewById(R.id.playerField);
        _timerfield = (TextView) findViewById(R.id.timerField);
        _winrecordfield = (TextView) findViewById(R.id.winrecordfield);
        _lossrecordfield = (TextView) findViewById(R.id.lossrecordfield);
        _opponentnamefield = (TextView) findViewById(R.id.opponentNameField);
        _numpplfield = (TextView) findViewById(R.id.numPplField);

        _quitbutton = (Button) findViewById(R.id.quitButton);
        _quitbutton.setClickable(false);

        Game.NotInGame.set(true);

        Thread t = new Thread() {
            public void run() {
                while (true) {
                    _board = new Board();
                    _c = new Client();
                    _game = new Game(_ttt, _c, _board);
                    _ttt.setTurnfieldText("");
                    _ttt.setPlayerfieldText("");
                    _ttt.setTimerfieldText("");
                    _ttt.setQuitbuttonVisible(false);
                    _ttt.setNumPplText("");
                    _ttt.setOpponentText("");
                    _ttt.setWinfieldText("");
                    _ttt.setLossfieldText("");
                    _display.init(_game.getBoard(), _ttt);

                    _game.start(_username, _password);
                    _ttt.repaintDisplay();

                    while (Game.NotInGame.get())
                        ;

                    _c.close();
                }
            }
        };
        t.start();
    }

    /** Called when Quit button is clicked */
    public void exitApp(View view) {
        if (!_c.getDoneInit()) {
            setPlayerfieldText("Quitting...");
            synchronized (this) {
                _c.close();
            }
        }
        setPlayerfieldText("");
        Game.NotInGame.set(true);
        setGameOverMsg(ITicTacToe.CLICK_TO_START);
        setQuitbuttonVisible(false);
        if (!_c.getDoneInit())
            System.exit(0);
    }

    /** Called when Start button is clicked */
    public void startApp(View view) {
        synchronized (this) {
            if (getGameOverMsg() != null) {
                Game.NotInGame.set(false);
                setGameOverMsg(null);
            }
        }
    }

    public void onBackPressed() {
        if (!_c.getDoneInit()) {
            synchronized (this) {
                _c.close();
            }
        }
        Game.NotInGame.set(true);
        setGameOverMsg(ITicTacToe.CLICK_TO_START);
        setQuitbuttonVisible(false);
        System.exit(0);
    }


    public void setTurnfieldText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _turnfield.setText(text);
            }
        });
    }

    public void setPlayerfieldText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _playerfield.setText(text);
            }
        });
    }

    public void setTimerfieldText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _timerfield.setText(text);
            }
        });
    }

    public void setWinfieldText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _winrecordfield.setText(text);
            }
        });
    }

    public void setLossfieldText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _lossrecordfield.setText(text);
            }
        });
    }

    public void setOpponentText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _opponentnamefield.setText(text);
            }
        });
    }

    public void setNumPplText(final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _numpplfield.setText(text);
            }
        });
    }

    public synchronized void repaintDisplay() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _display.invalidate();
            }
        });
    }

    public void handlePlayerLogin(String[] args) {

    }

    public void showGameOverDialog(final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _numpplfield.setText(message);
            }
        });
    }

    public void setQuitbuttonVisible(final boolean enable) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                _quitbutton.setClickable(enable);
            }
        });
    }

    public synchronized void setGameOverMsg(String newMsg) {
        _gameOverMsg = newMsg;
    }

    public synchronized String getGameOverMsg() {
        return _gameOverMsg;
    }

    public int getInput(char symbol) {
        return _display.getInput(symbol);
    }

    public void runGame() {}

    public void setStartbuttonVisible(boolean enable) {}
}
