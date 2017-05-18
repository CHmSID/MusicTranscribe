# Sound Analysis using Fourier Transform
Thursday February 9th

My application will make use of an algorithm called Fourier Transform, and more specifically, Short-Time Fast Fourier Transform, in order to convert short pieces of audio from time domain into frequency domain.

## The plan for frequency analysis
1. Collect a window of audio data (it will have to be an array of a power of 2).
2. Multiply the data with the window function (most probably I'll use the Hann or Hamming window for this step). This will average out the waveform on the edges of the window. I will have to make it optional as it'll be quite resource intensive to do in real time (30 fps or 33 milliseconds per refresh).
3. Perform an FFT on the data. The data that I'll receive from this step is still not usable yet though.
4. Now I have the data about individual frequency powers in arbitrary units. I will have to map them to each piano key. This is a little tough as low piano keys have small differences in frequency, while high piano keys have large differences, ie: the relation is not linear.

## The plan for audio slowdown
It's counter-intuitive, but interpolating data in time domain will not slow down the music, even though there's twice as much data to go through.
Well, it _will_ slow it down, but it'll also halve the pitch of the audio, which is very undesirable in a music transcription software.

The application must slow the music down, without loosing the pitch information of the original. To achieve that, I need to interpolate the data in the frequency domain, I think. So here's the plan:

1. Perform all the steps as above up to running FFT through the data.
2. Interpolate the data I receive from step 1. I think linear interpolation should suffice, as the data is very dense.
3. Now I run it through an Inverse Fast Fourier Transform. During this step, the interpolated data is converted back into time domain and will become available to the audio library for playing.

Even though it's 'fast', FFT is still very slow and I'm not sure I will be able to perform both frequency analysis and music slowdown at once. I'm considering providing and option to pre-compute a small piece of audio (around 10 seconds).

# Rendering the waveform in OpenGL
Tuesday March 14th

I am using modern OpenGL (Core 3.3) for all my rendering needs.

A waveform is very useful in audio analysis. It's multi-purpose because it can act as a seek bar like in normal audio players, as well as showing the user peaks of audio, which help in determining the structure of the song (8 drum kicks per bar, for example).
A waveform consists of stems, where each stem represents the average amplitude of a group of audio samples. A naive way would be to draw each stem as a vertical line. This would result in thousands of draw calls each frame for an average-sized music file.
Each stem is a line. Each line consists of 2 vertices (points). That's potentially a million vertices that need to be transfered from CPU to the GPU.How to decrease the number of draw calls in this case? I could render to texture (image).

I could draw each stem on an in-memory texture, then plaster that texture on a single surface (rectangle). This way I only have to draw each stem once, as opposed to drawing them on each frame. Additional bonus points for reducing the number of vertices from a million to 6 (a surface is made of 2 triangles).
But this method will never work, or maybe it will, in a far away future. This is because unless the music is really short, the number of stems will far exceed the texture size on any modern graphics card (nowadays usually 4096 pixels).
That's only 4096 stems, if we average 750 samples per stem, that comes out to 3072000 samples. That's only 35 seconds of a stereo audio sampled at 44.1KHz (standard for digital audio)!

Compromise! Divide the texture into smaller "chunks" of, let's say 512 pixels, and render them together side by side. That comes to rendering 6 surfaces on the widest of screens at the same time, using only 36 vertices. Still better than a million.

# Tone transposition
Thursday May 18th

Transposing tone is the polar opposite of stretching audio. That is, how to change the tone of the song to, for example, fit the key of an instrument we're transcribing for, but without changing the speed of the song? The answer is suprisingly simple once we have the time stretching implemented!
Time and tone are directly proportional to each other and one can not change without other changing, when using naive methods. That means, if the speed doubles, the tone rises by an octave, ie: 12 tones. If we slow the music down to half its speed, the tonality lowers by 12 tones.
I am hoping to exploit that relationship between tone and speed.

If I stretch the song by a factor fo 2, it will be twice as long, but in the same key. If that stretched song is then sped up to twice its speed, the resulting length of the song will be its original length, but the song will be 12 tones higher! That's how you would transpose a song by 12 tones.
Because the relationship is linear, I am able to use simple algebra to calculate the stretch factor and play speed to achieve a 1 tone transposition. Let's assume we're transcribing for a saxophone and do a simple example of transposing a song by 3 tones down:

Let's do some maths!
The speed of the song is determined by its sample rate, in other words, how many samples are being played per second. Usually this number is 44100 for normal audio files and 48000 for CDs. Lets assume ours is 44100 samples per second.
This means if we played that song at a speed of 88200 samples per second, we'd be playing it twice as fast. 22050 would be twice as slow.

1. First, we need to know the sample rate at which we should be playing the song. We want three tones up, so we need to add 3/12ths (12 tones in octave) to our original sample rate. This comes out to 47775 samples per second.
2. By performing 47775/44100 we receive 1.083. This is our slowdown factor.

That's all! We need to stretch our music to be 1.083 slower, and play it at a sample rate of 47775 to receive a song at its original speed but transposed 3 tones up!
