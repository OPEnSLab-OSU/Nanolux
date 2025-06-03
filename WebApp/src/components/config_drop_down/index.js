import { useSignal } from '@preact/signals';
import { useEffect } from 'preact/hooks';
import style from '../patterns/style.css';

/**
 * @brief A UI element that displays a dropdown menu of configuration options 
 *        based on the selected pattern index.
 *
 * @param patternIdx    The index of the currently selected pattern, used to 
 *                      choose which set of configuration options to show.
 * @param initial       The initial selected configuration index.
 * @param structureRef  The string reference where the selected configuration 
 *                      index should be stored.
 * @param update        A function to update an external data structure with 
 *                      the new configuration index.
 *
 * @returns The ConfigDropDown UI element.
 */
const ConfigDropDown = ({
    patternIdx,
    initial,
    structureRef,
    update
}) => {
    
    // Predefined configuration options for each pattern index.
    const configs = [
        ["None"],
        ["Linear", "Blended", "Logarithmic"],
        ["Default", "Falling Bars"],
        ["Default"],
        ["Default"],
        ["Default"],
        ["Default"],
        ["Default"],
        ["Default", "Blur", "Sine"],
        ["Default", "Octaves", "Shift", "Compression"],
        ["Default", "Shift"],
        ["Hue", "Moving"],
        ["Talk", "Sections"],
        ["Default"],
        ["Volume", "Frequency"],
        ["Default"],
        ["Default"],
        ["Default"],
        ["Default"],
        ["Default"]
    ]

    // Signal to hold the current selected configuration index.
    const current = useSignal(initial);

    /**
     * @brief Ensures that when the pattern index changes, the current selected
     *        configuration index is still valid. If it's out of range, reset to 0.
     */
    useEffect(() => {
        if (current.value >= (configs[patternIdx]?.length ?? 0)) {
            current.value = 0;
            update(structureRef, 0);
        }
    }, [patternIdx]);

    /**
     * @brief Handles changes to the dropdown by updating the internal signal 
     *        and the external data structure.
     *
     * @param e  The change event from the <select> element, containing the 
     *           new selected index in `e.target.value`.
     */
    const handleChange = e => {
        const idx = Number(e.target.value);
        current.value = idx;
        update(structureRef, idx);
    }

    return (
        <div>
            <label className={style.label} htmlFor="config-options">Config</label>
            <select
                id="config-options"
                className={style.select}
                value={current.value}
                onChange={handleChange}
            >
                {(configs[patternIdx] ?? configs[0]).map((optionName, i) => (
                    <option key={i} value={i}>{optionName}</option>
                ))}
            </select>
        </div>
    );
}

export default ConfigDropDown;