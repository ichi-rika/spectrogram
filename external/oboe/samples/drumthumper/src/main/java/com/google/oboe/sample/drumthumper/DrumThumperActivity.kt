/*
 * Copyright 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.mobileer.drumthumper

import android.content.Context
import android.media.AudioDeviceCallback
import android.media.AudioDeviceInfo
import android.media.AudioManager
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.Toast

import androidx.appcompat.app.AppCompatActivity

import java.util.*

import java.time.LocalDateTime;

import kotlin.concurrent.schedule
import kotlin.math.roundToInt

class DrumThumperActivity : AppCompatActivity(),
        TriggerPad.DrumPadTriggerListener,
        SeekBar.OnSeekBarChangeListener,
        View.OnClickListener {
    private val TAG = "DrumThumperActivity"

    private var mAudioMgr: AudioManager? = null

    private var mDrumPlayer = DrumPlayer()

    private val mUseDeviceChangeFallback = false
    private val mSwitchTimerMs = 500L

    private var mDevicesInitialized = false

    private var mDeviceListener: DeviceListener = DeviceListener()

    private var mMixControlsShowing = false;

    init {
        // Load the library containing the a native code including the JNI  functions
        System.loadLibrary("drumthumper")
    }

    inner class DeviceListener: AudioDeviceCallback() {
        fun logDevices(label: String, devices: Array<AudioDeviceInfo> ) {
            Log.i(TAG, label + " " + devices.size)
            for(device in devices) {
                Log.i(TAG, "  " + device.getProductName().toString()
                    + " type:" + device.getType()
                    + " source:" + device.isSource()
                    + " sink:" + device.isSink())
            }
        }

        override fun onAudioDevicesAdded(addedDevices: Array<AudioDeviceInfo> ) {
            // Note: This will get called when the callback is installed.
            if (mDevicesInitialized) {
                logDevices("onAudioDevicesAdded", addedDevices)
                // This is not the initial callback, so devices have changed
                Toast.makeText(applicationContext, "Added Device", Toast.LENGTH_LONG).show()
                resetOutput()
            }
            mDevicesInitialized = true
        }

        override fun onAudioDevicesRemoved(removedDevices: Array<AudioDeviceInfo> ) {
            logDevices("onAudioDevicesRemoved", removedDevices)
            Toast.makeText(applicationContext, "Removed Device", Toast.LENGTH_LONG).show()
            resetOutput()
        }

        private fun resetOutput() {
            Log.i(TAG, "resetOutput() time:" + LocalDateTime.now() + " native reset:" + mDrumPlayer.getOutputReset());
            if (mDrumPlayer.getOutputReset()) {
                // the (native) stream has been reset by the onErrorAfterClose() callback
                mDrumPlayer.clearOutputReset()
            } else {
                // give the (native) stream a chance to close it.
                val timer = Timer("stream restart timer time:" + LocalDateTime.now(),
                        false)
                // schedule a single event
                timer.schedule(mSwitchTimerMs) {
                    if (!mDrumPlayer.getOutputReset()) {
                        // still didn't get reset, so lets do it ourselves
                        Log.i(TAG, "restartStream() time:" + LocalDateTime.now())
                        mDrumPlayer.restartStream()
                    }
                }
            }
        }
    }

    //
    // UI Helpers
    //
    val GAIN_FACTOR = 100.0f;
    val MAX_PAN_POSITION = 200.0f;
    val HALF_PAN_POSITION = MAX_PAN_POSITION / 2.0f

    fun gainPosToGainVal(pos: Int) : Float {
        // map 0 -> 200 to 0.0f -> 2.0f
        return pos.toFloat() / GAIN_FACTOR
    }

    fun gainValToGainPos(value: Float) : Int {
        return (value * GAIN_FACTOR).toInt()
    }

    fun panPosToPanVal(pos: Int) : Float {
        // map 0 -> 200 to -1.0f -> 1..0f
        return (pos.toFloat() - HALF_PAN_POSITION) / HALF_PAN_POSITION
    }

    fun panValToPanPos(value: Float) : Int {
        // map -1.0f -> 1.0f to 0 -> 200
        return ((value * HALF_PAN_POSITION) + HALF_PAN_POSITION).toInt()
    }

    fun showMixControls(show : Boolean) {
        mMixControlsShowing = show;
        var showFlag = if (mMixControlsShowing) View.VISIBLE else View.GONE;
        findViewById<LinearLayout>(R.id.kickMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.snareMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.hihatOpenMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.hihatClosedMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.midTomMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.lowTomMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.rideMixControls).setVisibility(showFlag)
        findViewById<LinearLayout>(R.id.crashMixControls).setVisibility(showFlag)

        findViewById<Button>(R.id.mixCtrlBtn).setText(
                if (mMixControlsShowing) "Hide Mix Controls" else "Show Mix Controls")
    }

    fun connectMixSliders(panSliderId : Int, gainSliderId : Int, drumIndex : Int) {
        var panSeekbar = findViewById<SeekBar>(panSliderId)
        panSeekbar.setOnSeekBarChangeListener(this)
        panSeekbar.setProgress(panValToPanPos(mDrumPlayer.getPan(drumIndex)))

        var gainSeekbar = findViewById<SeekBar>(gainSliderId)
        gainSeekbar.setOnSeekBarChangeListener(this)
        gainSeekbar.setProgress(gainValToGainPos(mDrumPlayer.getGain(drumIndex)))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        mAudioMgr = getSystemService(Context.AUDIO_SERVICE) as AudioManager

        // mDrumPlayer.allocSampleData()
        mDrumPlayer.loadWavAssets(getAssets())
    }

    override fun onStart() {
        super.onStart()

        mDrumPlayer.setupAudioStream()

        if (mUseDeviceChangeFallback) {
            mAudioMgr!!.registerAudioDeviceCallback(mDeviceListener, null)
        }
    }

    override fun onResume() {
        super.onResume()

        // UI
        setContentView(R.layout.drumthumper_activity)

        // "Kick" drum
        findViewById<TriggerPad>(R.id.kickPad).addListener(this)
        connectMixSliders(R.id.kickPan, R.id.kickGain, DrumPlayer.BASSDRUM)

        // Snare drum
        findViewById<TriggerPad>(R.id.snarePad).addListener(this)
        connectMixSliders(R.id.snarePan, R.id.snareGain, DrumPlayer.SNAREDRUM)

        // Mid tom
        findViewById<TriggerPad>(R.id.midTomPad).addListener(this)
        connectMixSliders(R.id.midTomPan, R.id.midTomGain, DrumPlayer.MIDTOM)

        // Low tom
        findViewById<TriggerPad>(R.id.lowTomPad).addListener(this)
        connectMixSliders(R.id.lowTomPan, R.id.lowTomGain, DrumPlayer.LOWTOM)

        // Open hihat
        findViewById<TriggerPad>(R.id.hihatOpenPad).addListener(this)
        connectMixSliders(R.id.hihatOpenPan, R.id.hihatOpenGain, DrumPlayer.HIHATOPEN)

        // Closed hihat
        findViewById<TriggerPad>(R.id.hihatClosedPad).addListener(this)
        connectMixSliders(R.id.hihatClosedPan, R.id.hihatClosedGain, DrumPlayer.HIHATCLOSED)

        // Ride cymbal
        findViewById<TriggerPad>(R.id.ridePad).addListener(this)
        connectMixSliders(R.id.ridePan, R.id.rideGain, DrumPlayer.RIDECYMBAL)

        // Crash cymbal
        findViewById<TriggerPad>(R.id.crashPad).addListener(this)
        connectMixSliders(R.id.crashPan, R.id.crashGain, DrumPlayer.CRASHCYMBAL)

        findViewById<Button>(R.id.mixCtrlBtn).setOnClickListener(this)
        showMixControls(false);
    }

    override fun onPause() {
        super.onPause()
    }

    override fun onStop() {
        if (mUseDeviceChangeFallback) {
            mAudioMgr!!.unregisterAudioDeviceCallback(mDeviceListener)
        }

        mDrumPlayer.teardownAudioStream()

        super.onStop()
    }

    override fun onDestroy() {
        mDrumPlayer.unloadWavAssets();
        super.onDestroy()
    }

    //
    // DrumPad.DrumPadTriggerListener
    //
    override fun triggerDown(pad: TriggerPad) {
        // trigger the sound based on the pad
        when (pad.id) {
            R.id.kickPad -> mDrumPlayer.trigger(DrumPlayer.BASSDRUM)
            R.id.snarePad -> mDrumPlayer.trigger(DrumPlayer.SNAREDRUM)
            R.id.midTomPad -> mDrumPlayer.trigger(DrumPlayer.MIDTOM)
            R.id.lowTomPad -> mDrumPlayer.trigger(DrumPlayer.LOWTOM)
            R.id.hihatOpenPad -> mDrumPlayer.trigger(DrumPlayer.HIHATOPEN)
            R.id.hihatClosedPad -> mDrumPlayer.trigger(DrumPlayer.HIHATCLOSED)
            R.id.ridePad -> mDrumPlayer.trigger(DrumPlayer.RIDECYMBAL)
            R.id.crashPad -> mDrumPlayer.trigger(DrumPlayer.CRASHCYMBAL)
        }
    }

    override fun triggerUp(pad: TriggerPad) {
        // NOP
    }

    //
    // SeekBar.OnSeekBarChangeListener
    //
    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
        when (seekBar!!.id) {
            // BASSDRUM
            R.id.kickGain -> mDrumPlayer.setGain(DrumPlayer.BASSDRUM, gainPosToGainVal(progress))
            R.id.kickPan -> mDrumPlayer.setPan(DrumPlayer.BASSDRUM, panPosToPanVal(progress))

            // SNAREDRUM
            R.id.snareGain -> mDrumPlayer.setGain(DrumPlayer.SNAREDRUM, gainPosToGainVal(progress))
            R.id.snarePan -> mDrumPlayer.setPan(DrumPlayer.SNAREDRUM, panPosToPanVal(progress))

            // MIDTOM
            R.id.midTomGain -> mDrumPlayer.setGain(DrumPlayer.MIDTOM, gainPosToGainVal(progress))
            R.id.midTomPan -> mDrumPlayer.setPan(DrumPlayer.MIDTOM, panPosToPanVal(progress))

            // LOWTOM
            R.id.lowTomGain -> mDrumPlayer.setGain(DrumPlayer.LOWTOM, gainPosToGainVal(progress))
            R.id.lowTomPan -> mDrumPlayer.setPan(DrumPlayer.LOWTOM, panPosToPanVal(progress))

            // HIHATOPEN
            R.id.hihatOpenGain -> mDrumPlayer.setGain(DrumPlayer.HIHATOPEN, gainPosToGainVal(progress))
            R.id.hihatOpenPan -> mDrumPlayer.setPan(DrumPlayer.HIHATOPEN, panPosToPanVal(progress))

            // HIHATCLOSED
            R.id.hihatClosedGain -> mDrumPlayer.setGain(DrumPlayer.HIHATCLOSED, gainPosToGainVal(progress))
            R.id.hihatClosedPan -> mDrumPlayer.setPan(DrumPlayer.HIHATCLOSED, panPosToPanVal(progress))

            // RIDECYMBAL
            R.id.rideGain -> mDrumPlayer.setGain(DrumPlayer.RIDECYMBAL, gainPosToGainVal(progress))
            R.id.ridePan -> mDrumPlayer.setPan(DrumPlayer.RIDECYMBAL, panPosToPanVal(progress))

            // CRASHCYMBAL
            R.id.crashGain -> mDrumPlayer.setGain(DrumPlayer.CRASHCYMBAL, gainPosToGainVal(progress))
            R.id.crashPan -> mDrumPlayer.setPan(DrumPlayer.CRASHCYMBAL, panPosToPanVal(progress))
        }
    }

    override fun onStartTrackingTouch(seekBar: SeekBar?) {
        // NOP
    }

    override fun onStopTrackingTouch(seekBar: SeekBar?) {
        // NOP
    }

    override fun onClick(v: View?) {
        showMixControls(!mMixControlsShowing)
    }

}
