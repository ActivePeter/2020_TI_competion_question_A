package tv.higlobal.tcpdemo;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.drawable.Drawable;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.view.View;

/**
 * TODO: document your custom view class.
 */
public class WaveView extends View {
    private String mExampleString; // TODO: use a default from R.string...
    private int mExampleColor = Color.GREEN; // TODO: use a default from R.color...
    private float mExampleDimension = 0; // TODO: use a default from R.dimen...
    private Drawable mExampleDrawable;
    WaveView waveView;
    private Paint mLinePaint;
    private Paint mLineGreenPaint;

    private TextPaint mTextPaint;
    private float mTextWidth;
    private float mTextHeight;

    public enum WaveKind {
        Attitude,
        Heart,
        Temperature
    }

    public WaveKind currentWave = WaveKind.Attitude;

    public WaveView(Context context) {
        super(context);
        init(null, 0);
    }

    public WaveView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(attrs, 0);
    }

    public WaveView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init(attrs, defStyle);
    }

    class DataBuff {
        DataBuff(int buffLen,int xscale1) {
            len=buffLen;
            xscale=xscale1;
            dataBuffer = new float[len];
        }

        public void push(float a) {

            lastStep = step;
            step++;
            if (step == len) {
                step = 0;
            }
            if (a > dataBuffer[maxDataIndex]) {
                maxDataIndex = step;
            }else{
                dataBuffer[step] = a;
                maxDataIndex=findMaxIndex();
            }
            if (a < dataBuffer[minDataIndex]) {
                minDataIndex = step;
            }else{
                dataBuffer[step] = a;
                minDataIndex=findMinIndex();
            }
            dataBuffer[step] = a;
        }
        private int findMaxIndex(){
            int maxindex=0;
            for(int i=1;i<len;i++){
                if(dataBuffer[i]>dataBuffer[maxindex]){
                    maxindex=i;
                }
            }
            return maxindex;
        }
        private int findMinIndex(){
            int minindex=0;
            for(int i=1;i<len;i++){
                if(dataBuffer[i]<dataBuffer[minindex]){
                    minindex=i;
                }
            }
            return minindex;
        }
        public float getMinValue() {
            return dataBuffer[minDataIndex];
        }

        public float getMaxValue() {
            return dataBuffer[maxDataIndex];
        }

        int maxDataIndex = 0;
        int minDataIndex = 0;
        int len = 125;
        float[] dataBuffer;
        int step = 0;
        int lastStep = 0;
        int xscale;
    }

    public DataBuff DataBuff_Heart = new DataBuff(125,8);
    public DataBuff DataBuff_Attitude = new DataBuff(250,4);
    public DataBuff DataBuff_Temp = new DataBuff(250,4);

    public DataBuff getCurBuff() {
        return DataBuff_getDataBuffByType(currentWave);
    }

    public DataBuff DataBuff_getDataBuffByType(WaveKind a) {
        if (a == WaveKind.Attitude) {
            return DataBuff_Attitude;
        } else if (a == WaveKind.Temperature) {
            return DataBuff_Temp;
        } else {
            return DataBuff_Heart;
        }
    }

    //    final int dataBufferLen=2000;
//    float[] dataBuffer=new float[dataBufferLen];
//    int dataBufferStep=0;
//    int lastDataBufferStep=0;
//
//    public void pushValue(float a){
//        lastDataBufferStep=dataBufferStep;
//        dataBufferStep++;
//        if(dataBufferStep==dataBufferLen) {
//            dataBufferStep = 0;
//        }
//        dataBuffer[dataBufferStep]=a;
//    }
    public void setSelectedWave(WaveKind a) {
        currentWave = a;
    }

    private void init(AttributeSet attrs, int defStyle) {
        // Load attributes
        final TypedArray a = getContext().obtainStyledAttributes(
                attrs, R.styleable.WaveView, defStyle, 0);

        mExampleString = a.getString(
                R.styleable.WaveView_exampleString);
//        mExampleColor = a.getColor(
//                R.styleable.WaveView_exampleColor,
//                mExampleColor);
        // Use getDimensionPixelSize or getDimensionPixelOffset when dealing with
        // values that should fall on pixel boundaries.
        mExampleDimension = a.getDimension(
                R.styleable.WaveView_exampleDimension,
                mExampleDimension);

        if (a.hasValue(R.styleable.WaveView_exampleDrawable)) {
            mExampleDrawable = a.getDrawable(
                    R.styleable.WaveView_exampleDrawable);
            mExampleDrawable.setCallback(this);
        }

        a.recycle();

        // Set up a default TextPaint object
        mTextPaint = new TextPaint();
        mTextPaint.setFlags(Paint.ANTI_ALIAS_FLAG);
        mTextPaint.setTextAlign(Paint.Align.LEFT);
        mTextPaint.setTextSize(10);

        //set up line paint
        mLinePaint = new Paint();
        mLinePaint.setColor(Color.WHITE);
        mLinePaint.setStyle(Paint.Style.FILL);
        mLinePaint.setStrokeWidth(2);

        mLineGreenPaint = new Paint();
        mLineGreenPaint.setColor(Color.GREEN);
        mLineGreenPaint.setStyle(Paint.Style.FILL);
        mLineGreenPaint.setStrokeWidth(10);
        // Update TextPaint and text measurements from attributes
        invalidateTextPaintAndMeasurements();
    }

    private void invalidateTextPaintAndMeasurements() {
        mTextPaint.setTextSize(50);
        mTextPaint.setColor(mExampleColor);
        mTextWidth = mTextPaint.measureText(mExampleString);

        Paint.FontMetrics fontMetrics = mTextPaint.getFontMetrics();
        mTextHeight = fontMetrics.bottom;
    }

    float transformValueToRange(float a, float srcMin, float srcMax, float dstMin, float dstMax) {
        return dstMin + (a - srcMin) / (srcMax - srcMin) * (dstMax - dstMin);
    }

    @SuppressLint("DefaultLocale")
    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

        // TODO: consider storing these as member variables to reduce
        // allocations per draw cycle.
        int paddingLeft = getPaddingLeft();
        int paddingTop = getPaddingTop() + 150;//(getPaddingTop() + getPaddingBottom()) / 2;
        int paddingRight = getPaddingRight();
        int paddingBottom = getPaddingBottom() - 100;


        int contentWidth = getWidth() - paddingLeft - paddingRight;
        int contentHeight = getHeight() - paddingTop - paddingBottom;

        // Draw the text.
//        canvas.drawText(mExampleString,
//                paddingLeft + (contentWidth - mTextWidth) / 2,
//                paddingTop + (contentHeight + mTextHeight) / 2,
//                mTextPaint);


        DataBuff curBuf = DataBuff_getDataBuffByType(currentWave);
        float XScale = curBuf.xscale;
        for (int i = 0; i <=8; i++) {
            canvas.drawText(String.format("%.2f", (curBuf.getMinValue() + i / 8f * (curBuf.getMaxValue() - curBuf.getMinValue()))),
                    10,
                    paddingBottom + i / 8f * (paddingTop - paddingBottom),
                    mTextPaint);
        }

        for (int i = 2; i < curBuf.step; i++) {
            canvas.drawLine(
                    paddingLeft + (i - 1) * XScale,
                    transformValueToRange(curBuf.dataBuffer[i - 1], curBuf.getMinValue(), curBuf.getMaxValue(), paddingBottom, paddingTop),
                    paddingLeft + i * XScale,
                    transformValueToRange(curBuf.dataBuffer[i], curBuf.getMinValue(), curBuf.getMaxValue(), paddingBottom, paddingTop),
                    mLinePaint
            );
        }
        canvas.drawLine(
                paddingLeft + (curBuf.step) * XScale + 6,
                getPaddingTop(),
                paddingLeft + (curBuf.step) * XScale + 6,
                getPaddingBottom(),
                mLineGreenPaint
        );
        for (int i = curBuf.step + 2; i < curBuf.len; i++) {
            canvas.drawLine(
                    paddingLeft + (i - 1) * XScale,
                    transformValueToRange(curBuf.dataBuffer[i - 1], curBuf.getMinValue(), curBuf.getMaxValue(), paddingBottom, paddingTop),
                    paddingLeft + i * XScale,
                    transformValueToRange(curBuf.dataBuffer[i], curBuf.getMinValue(), curBuf.getMaxValue(), paddingBottom, paddingTop),
                    mLinePaint
            );
        }

        // Draw the example drawable on top of the text.
        if (mExampleDrawable != null) {
            mExampleDrawable.setBounds(paddingLeft, paddingTop,
                    paddingLeft + contentWidth, paddingTop + contentHeight);
            mExampleDrawable.draw(canvas);
        }
    }

    /**
     * Gets the example string attribute value.
     *
     * @return The example string attribute value.
     */
    public String getExampleString() {
        return mExampleString;
    }

    /**
     * Sets the view's example string attribute value. In the example view, this string
     * is the text to draw.
     *
     * @param exampleString The example string attribute value to use.
     */
    public void setExampleString(String exampleString) {
        mExampleString = exampleString;
        invalidateTextPaintAndMeasurements();
    }

    /**
     * Gets the example color attribute value.
     *
     * @return The example color attribute value.
     */
    public int getExampleColor() {
        return mExampleColor;
    }

    /**
     * Sets the view's example color attribute value. In the example view, this color
     * is the font color.
     *
     * @param exampleColor The example color attribute value to use.
     */
    public void setExampleColor(int exampleColor) {
        mExampleColor = exampleColor;
        invalidateTextPaintAndMeasurements();
    }

    /**
     * Gets the example dimension attribute value.
     *
     * @return The example dimension attribute value.
     */
    public float getExampleDimension() {
        return mExampleDimension;
    }

    /**
     * Sets the view's example dimension attribute value. In the example view, this dimension
     * is the font size.
     *
     * @param exampleDimension The example dimension attribute value to use.
     */
    public void setExampleDimension(float exampleDimension) {
        mExampleDimension = exampleDimension;
        invalidateTextPaintAndMeasurements();
    }

    /**
     * Gets the example drawable attribute value.
     *
     * @return The example drawable attribute value.
     */
    public Drawable getExampleDrawable() {
        return mExampleDrawable;
    }

    /**
     * Sets the view's example drawable attribute value. In the example view, this drawable is
     * drawn above the text.
     *
     * @param exampleDrawable The example drawable attribute value to use.
     */
    public void setExampleDrawable(Drawable exampleDrawable) {
        mExampleDrawable = exampleDrawable;
    }
}
