"""
task-11-blur-estimation-with-fourier-transform.py

>>> IMPORTANT <<<
Implement the function `frequency_blur_score` below.

Rules:
- Keep the function name and signature EXACTLY the same.
- Do NOT use any external network calls.
- You may ONLY use standard Python, NumPy, and OpenCV (cv2).
- Return a single float (higher = sharper OR lower = blurrier, but be consistent).

Tip (from the FFT blur-detection tutorial):
- Convert to grayscale
- 2D FFT -> shift DC to center
- Zero-out a centered square (low frequencies)
- Magnitude spectrum (e.g., log1p(abs(...)))
- Use the mean magnitude of the remaining spectrum as the score
"""

from typing import Union
import numpy as np
import cv2


def frequency_blur_score(
    image: Union[np.ndarray, "cv2.Mat"],
    center_size: int = 60
) -> float:
    """
    Compute a blur/sharpness score in the frequency domain.

    Parameters
    ----------
    image : np.ndarray
        Input image, grayscale or BGR. Any dtype accepted; will be converted to float32.
    center_size : int, default=60
        Side length of the central square (low-frequency) region to suppress.

    Returns
    -------
    float
        A scalar score. You should make it so that SHARPER images get a HIGHER score.
        (This will align with the grader's expectation.)
    """
    # ====== YOUR CODE STARTS HERE ======
    score = 0.0
    # ====== YOUR CODE ENDS HERE ======
    return score
