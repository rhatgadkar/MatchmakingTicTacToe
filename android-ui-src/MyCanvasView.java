package com.example.canvastry;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.MotionEvent;
import android.view.View;
import android.content.Context;
import android.util.AttributeSet;

/**
 * Created by rhatgadkar on 6/4/17.
 */

public class MyCanvasView extends View {

    Paint mP;
    static final int ROWS = 3;
    static final int COLS = 3;

    private Board _board;
    private ITicTacToe _ttt;

    private class InputMsg {
        public volatile int input;
    }

    private boolean _allowInput;
    private InputMsg _acceptedInput;
    private char _symbol;

    private class InputThread implements Runnable {
        private InputMsg _msg;
        public InputThread(InputMsg msg) {
            _msg = msg;
        }
        @Override
        public void run() {
            while (_msg.input == -1 && !Game.NotInGame.get())
                ;
        }
    }

    public int getInput(char symbol) {
        _acceptedInput.input = -1;
        Thread t = new Thread(new InputThread(_acceptedInput));
        t.start();

        _symbol = symbol;
        _allowInput = true;
        try {
            t.join();
        } catch (InterruptedException e) {
            System.err.println("Could not join input thread.");
            System.exit(1);
        }

        return _acceptedInput.input;
    }

    public MyCanvasView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mP = new Paint();
        mP.setColor(Color.BLACK);
        mP.setStyle(Paint.Style.STROKE);
    }

    public void init(Board b, ITicTacToe t) {
        _ttt = t;
        _board = b;
        _allowInput = false;
        _acceptedInput = new InputMsg();
        _acceptedInput.input = -1;
        _symbol = 0;
        if (Game.NotInGame.get())
            _ttt.setGameOverMsg(ITicTacToe.CLICK_TO_START);

        _ttt.repaintDisplay();
    }

    public void onDraw(Canvas c) {
        if (_ttt == null)
            return;
        synchronized (_ttt) {
            c.drawColor(Color.LTGRAY);
            int wDiv = c.getWidth() / COLS;
            int hDiv = c.getHeight() / ROWS;
            c.drawLine(wDiv, 0, wDiv, c.getHeight(), mP);
            c.drawLine(wDiv * 2, 0, wDiv * 2, c.getHeight(), mP);
            c.drawLine(0, hDiv, c.getWidth(), hDiv, mP);
            c.drawLine(0, hDiv * 2, c.getWidth(), hDiv * 2, mP);

            // draw board
            for (int row = 0; row < ROWS; row++) {
                for (int col = 0; col < COLS; col++) {
                    char symbol = _board.getSymbolAtCoord(row, col);
                    int x = col * wDiv;
                    int y = row * hDiv;

                    if (symbol == 'x') {
                        c.drawLine(x, y, x + wDiv, y + hDiv, mP);
                        c.drawLine(x + wDiv, y, x, y + hDiv, mP);
                    }
                    else if (symbol == 'o')
                        c.drawCircle(x + (wDiv / 2), y + (hDiv / 2), Math.min(wDiv / 2, hDiv / 2), mP);
                }
            }
        }
    }

    public boolean onTouchEvent(MotionEvent e) {
        if (_allowInput) {
            int row = (int) (e.getY() / (this.getHeight() / ROWS));
            int col = (int) (e.getX() / (this.getWidth() / COLS));
            if (row == ROWS)
                row = ROWS - 1;
            if (col == COLS)
                col = COLS - 1;

            if (col >= 0 && col < COLS && row >= 0 && row < ROWS) {
                int input;
                if (row == 0)
                    input = col + 1;
                else if (row == 1)
                    input = row * 4 + col;
                else
                    input = row * 3 + col + 1;
                boolean addTile = _board.insert(_symbol, input);
                if (addTile) {
                    _acceptedInput.input = input;
                    _allowInput = false;
                }
            }
        }

        return true;
    }


}
