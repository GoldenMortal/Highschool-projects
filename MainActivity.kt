/*
  MainActivity.kt - Minimal "Read Aloud" Companion App
  ------------------------------------------------------
  1. Fetches a JPEG photo from the ESP32-CAM's /capture endpoint over WiFi
  2. Runs on-device text recognition (Google ML Kit) on the photo
  3. Speaks the recognized text using Android's Text-to-Speech engine
     (audio automatically plays over a connected Bluetooth headset/earbuds
     once they're paired and connected as the phone's active audio route
     - no extra Bluetooth code needed, Android handles that routing)

  Required Gradle dependencies (add to app/build.gradle):
    implementation 'com.google.mlkit:text-recognition:16.0.0'
    implementation 'androidx.core:core-ktx:1.12.0'

  Required permissions (add to AndroidManifest.xml):
    <uses-permission android:name="android.permission.INTERNET" />
    <uses-permission android:name="android.permission.BLUETOOTH_CONNECT" />

  Layout (activity_main.xml) needs just one button:
    <Button android:id="@+id/captureButton" android:text="Capture & Read" ... />
*/

package com.example.readaloud

import android.os.Bundle
import android.speech.tts.TextToSpeech
import android.widget.Button
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.google.mlkit.vision.common.InputImage
import com.google.mlkit.vision.text.TextRecognition
import com.google.mlkit.vision.text.latin.TextRecognizerOptions
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.net.HttpURLConnection
import java.net.URL
import java.util.Locale

class MainActivity : AppCompatActivity() {

    // Change this to your ESP32-CAM's IP address (printed to Serial on boot)
    private val ESP32_CAPTURE_URL = "http://192.168.1.50/capture"

    private lateinit var tts: TextToSpeech

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tts = TextToSpeech(this) { status ->
            if (status == TextToSpeech.SUCCESS) {
                tts.language = Locale.getDefault()
            }
        }

        findViewById<Button>(R.id.captureButton).setOnClickListener {
            captureAndRead()
        }
    }

    private fun captureAndRead() {
        lifecycleScope.launch {
            try {
                val jpegBytes = withContext(Dispatchers.IO) { fetchImageBytes() }
                val bitmap = withContext(Dispatchers.Default) {
                    android.graphics.BitmapFactory.decodeByteArray(jpegBytes, 0, jpegBytes.size)
                }
                recognizeAndSpeak(bitmap)
            } catch (e: Exception) {
                Toast.makeText(this@MainActivity, "Error: ${e.message}", Toast.LENGTH_LONG).show()
            }
        }
    }

    // ---------- Fetch photo from ESP32 ----------
    private fun fetchImageBytes(): ByteArray {
        val connection = URL(ESP32_CAPTURE_URL).openConnection() as HttpURLConnection
        connection.connectTimeout = 5000
        connection.readTimeout = 5000
        connection.requestMethod = "GET"
        connection.connect()

        if (connection.responseCode != HttpURLConnection.HTTP_OK) {
            throw Exception("ESP32 returned code ${connection.responseCode}")
        }

        return connection.inputStream.readBytes()
    }

    // ---------- OCR + Speak ----------
    private fun recognizeAndSpeak(bitmap: android.graphics.Bitmap) {
        val image = InputImage.fromBitmap(bitmap, 0)
        val recognizer = TextRecognition.getClient(TextRecognizerOptions.DEFAULT_OPTIONS)

        recognizer.process(image)
            .addOnSuccessListener { visionText ->
                val recognizedText = visionText.text
                if (recognizedText.isBlank()) {
                    Toast.makeText(this, "No text found in image", Toast.LENGTH_SHORT).show()
                } else {
                    tts.speak(recognizedText, TextToSpeech.QUEUE_FLUSH, null, null)
                }
            }
            .addOnFailureListener { e ->
                Toast.makeText(this, "OCR failed: ${e.message}", Toast.LENGTH_LONG).show()
            }
    }

    override fun onDestroy() {
        tts.stop()
        tts.shutdown()
        super.onDestroy()
    }
}
