# Processing in the Frequency Domain — Blur Detection (FFT)

In this task, you'll implement a blur/sharpness score using the **2D Fast Fourier Transform (FFT)**. The idea: **sharp images contain more high-frequency content**. We'll suppress low frequencies around the DC component and summarize what's left.

> Reference tutorial: [*OpenCV Fast Fourier Transform (FFT) for blur detection in images and video streams* (PyImageSearch).](https://pyimagesearch.com/2020/06/15/opencv-fast-fourier-transform-fft-for-blur-detection-in-images-and-video-streams)

## What you must do

1. Open `student_fft.py`.
2. Implement the function:

```python
def frequency_blur_score(image: np.ndarray, center_size: int = 60) -> float:
    ...
