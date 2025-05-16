import { h } from 'preact';
import style from './style.css';

/**
 * @brief A modal that gives the user information on the patterns featured on the device.
 * 
 * @param isOpen If the modal should be shown to the user
 * @param onClose The function to be executed when the modal is clicked.
 */
const PatternModal = ({ isOpen, onClose}) => {
    if (!isOpen) {
        return null;
    }

    return (
        <div className={style.modalOverlay}>
            <div className={style.modal}>
                <h2 className={style.modalTitle}>Patterns</h2>
                <h3>Only 1-4 patterns can exist in one profile</h3>
                <div className={style.TextContainer}>
                    <h3>Pixel Frequency</h3>
                    <p>A Pixel will change position based on volume.
                    </p>
                    <h3>Confetti</h3>
                    <p>Colored speckles will blink and fade smoothly across the board. 
                    </p>
                    <h3>Hue Trail</h3>
                    <p>Displays a moving stream of lights.
                    </p>
                    <h3>Saturated</h3>
                    <p>Fills the strip with a variety of colors that shift over time.
                    </p>
                    <h3>Groovy</h3>
                    <p>Fluctuates in waves of green, yellow, purple, and blue.
                    </p>
                    <h3>Talking</h3>
                    <p>Generates three clusters: one in the middle, and two that travel back and forth from the center.
                    </p>
                    <h3>Glitch</h3>
                    <p>Creates two light clusters that move in a sine wave motion, with speed being based on volume.
                        This pattern is currently not working.
                    </p>
                    <h3>Equalizer</h3>
                    <p>Displays lights on the strip equally.
                    </p>
                    <h3>Bands</h3>
                    <p>Splits the strip into five bands of different colors. This pattern is currently not working.
                    </p>
                    <h3>Eq</h3>
                    <p>Changes the brightness of the pixels on the strip based on volume. Hue is locked to a rainbow.
                    </p>
                    <h3>Raindrop</h3>
                    <p>A random spot on the strip will ripple based on the frequency.
                    </p>
                    <h3>Tug of War</h3>
                    <p>The strip splits into two sides that move back and forth.
                    </p>
                    <h3>Fire 2012</h3>
                    <p>Similar to Raindrop but with fire-like colors.
                    </p>
                    <h3>Bar Fill</h3>
                    <p>Flashes the whole LED strip with the selected colors
                    </p>
                    <h3>Vowels Raindrop</h3>
                    <p>A variation of the Raindrop pattern.
                    </p>
                    <h3>Blend in</h3>
                    <p>Flashes the whole LED strip with two colors that bleed together.
                    </p>
                    <h3>Bleed Through</h3>
                    <p>Sends different colors down the LED strip that transitions based on volume.
                    </p>
                    <h3>Showcase Percussion</h3>
                    <p>Sends a color down the strip if a percussion sound is detected.
                    </p>
                    <h3>Showcase Centroid</h3>
                    <p>Divides the LED strip depending on the frequency of the Centroid.
                    </p>
                    <h3>String Theory</h3>
                    <p>Maps the LED strip to imitate the strings of a violin, changing position and color based on what strings are detected.
                    </p>
                    <h3>Splash Theory</h3>
                    <p>Simulates a splash effect based on volume.
                    </p>
                    <h3>Delta Heat</h3>
                    <p>A heat map of the frequency bins based on amplitude change.
                    </p>
                </div>
                <button className={style.modalButton} onClick={onClose}>
                    Dismiss
                </button>
            </div>
        </div>
    );
};

export default PatternModal;
