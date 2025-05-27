import style from './style.css';


const PATTERN_INFOS = [
  { title: 'Pixel Frequency',      desc: 'A Pixel will change position based on volume.' },
  { title: 'Confetti',             desc: 'Colored speckles will blink and fade smoothly across the board.' },
  { title: 'Hue Trail',            desc: 'Displays a moving stream of lights.' },
  { title: 'Saturated',            desc: 'Fills the strip with a variety of colors that shift over time.' },
  { title: 'Groovy',               desc: 'Fluctuates in waves of green, yellow, purple, and blue.' },
  { title: 'Talking',              desc: 'Generates three clusters: one in the middle, and two that travel back and forth from the center.' },
  { title: 'Glitch',               desc: 'Creates two light clusters that move in a sine wave motion, with speed being based on volume. This pattern is currently not working.' },
  { title: 'Equalizer',            desc: 'Displays lights on the strip equally.' },
  { title: 'Bands',                desc: 'Splits the strip into five bands of different colors. This pattern is currently not working.' },
  { title: 'Eq',                   desc: 'Changes the brightness of the pixels on the strip based on volume. Hue is locked to a rainbow.' },
  { title: 'Raindrop',             desc: 'A random spot on the strip will ripple based on the frequency.' },
  { title: 'Tug of War',           desc: 'The strip splits into two sides that move back and forth.' },
  { title: 'Fire 2012',            desc: 'Similar to Raindrop but with fire-like colors.' },
  { title: 'Bar Fill',             desc: 'Flashes the whole LED strip with the selected colors.' },
  { title: 'Vowels Raindrop',      desc: 'A variation of the Raindrop pattern.' },
  { title: 'Blend in',             desc: 'Flashes the whole LED strip with two colors that bleed together.' },
  { title: 'Bleed Through',        desc: 'Sends different colors down the LED strip that transitions based on volume.' },
  { title: 'Showcase Percussion',  desc: 'Sends a color down the strip if a percussion sound is detected.' },
  { title: 'Showcase Centroid',    desc: 'Divides the LED strip depending on the frequency of the Centroid.' },
  { title: 'String Theory',        desc: 'Maps the LED strip to imitate the strings of a violin, changing position and color based on what strings are detected.' },
  { title: 'Splash Theory',        desc: 'Simulates a splash effect based on volume.' },
  { title: 'Delta Heat',           desc: 'A heat map of the frequency bins based on amplitude change.' },
];

/**
 * @brief A modal that gives the user information on the patterns featured on the device.
 * 
 * @param isOpen If the modal should be shown to the user
 * @param onClose The function to be executed when the modal is clicked.
 */
const PatternModal = ({ isOpen, onClose }) => {
  if (!isOpen) return null;

  return (
    <div role='presentation' className={style.modalOverlay}>
      <div
        className={style.modal}
        role='dialog'
        aria-modal='true'
        aria-labelledby='patterns-modal-title'
        aria-describedby='patterns-modal-desc'
        tabIndex={-1}
      >
        <h2 id='patterns-modal-title' className={style.modalTitle}>Patterns</h2>
        <p id='patterns-modal-desc' className={style.modalSubtitle}>
            <a
              href='https://github.com/OPEnSLab-OSU/Nanolux/wiki/Pattern-Library'
              target='_blank'
              rel='noopener noreferrer'
              >View more information about the full pattern library on our GitHub wiki
            </a>
        </p>
        <div role='region' aria-label='Available patterns' className={style.patternsGrid}>
          {PATTERN_INFOS.map(({ title, desc }) => (
            <div key={title} className={style.patternItem} role='group' aria-labelledby={`pattern-${title}-title`}>
              <h3 id={`pattern-${title}-title`}>{title}</h3>
              <p>{desc}</p>
            </div>
          ))}
        </div>
        <button className={style.modalButton} onClick={onClose} aria-label='Dismiss patterns dialog'>
          Dismiss
        </button>
      </div>
    </div>
  );
};

export default PatternModal;