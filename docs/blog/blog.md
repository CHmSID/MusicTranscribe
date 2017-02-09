#Sound Analysis using Fourier Transform
My application will make use of an algorithm called Fourier Transform, and more specifically, Short-Time Fast Fourier Transform, in order to convert short pieces of audio from time domain into frequency domain.

##The plan for frequency analysis
1. Collect a window of audio data (it will have to be an array of a power of 2).
2. Multiply the data with the window function (most probably I'll use the Hann or Hamming window for this step). This will average out the waveform on the edges of the window. I will have to make it optional as it'll be quite resource intensive to do in real time (30 fps or 33 milliseconds per refresh).
3. Perform an FFT on the data. The data that I'll receive from this step is still not usable yet though.
4. Now I have the data about individual frequency powers in arbitrary units. I will have to map them to each piano key. This is a little tough as low piano keys have small differences in frequency, while high piano keys have large differences, ie: the relation is not linear.

##The plan for audio slowdown
It's counter-intuitive, but interpolating data in time domain will not slow down the music, even though there's twice as much data to go through.
Well, it _will_ slow it down, but it'll also halve the pitch of the audio, which is very undesirable in a music transcription software.

The application must slow the music down, without loosing the pitch information of the original. To achieve that, I need to interpolate the data in the frequency domain, I think. So here's the plan:

1. Perform all the steps as above up to running FFT through the data.
2. Interpolate the data I receive from step 1. I think linear interpolation should suffice, as the data is very dense.
3. Now I run it through an Inverse Fast Fourier Transform. During this step, the interpolated data is converted back into time domain and will become available to the audio library for playing.

Even though it's 'fast', FFT is still very slow and I'm not sure I will be able to perform both frequency analysis and music slowdown at once. I'm considering providing and option to pre-compute a small piece of audio (around 10 seconds).

I will write about Equalizing in the next blog.