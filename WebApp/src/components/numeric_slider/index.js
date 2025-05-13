import {useSignal} from "@preact/signals";
import style from './style.css';
import { useEffect } from "preact/hooks";
import TooltipWrapper from "../tooltip/tooltip_wrapper";

/**
 * @brief A UI element that creates a draggable slider and an readout
 * showing the value set by the slider.
 * 
 * @param label The label that is displayed alongside the slider.
 * @param min The minimum value selectable by the slider.
 * @param max The maximum value selectable by the slider.
 * @param initial The initial value shown by the slider.
 * @param structure_ref The string reference to store values at.
 * @param update A function to update an external data structure.
 * 
 * @returns The UI element itself.
 */
const NumericSlider = ({
    label,
    tooltip,
    min,
    max,
    initial,
    structure_ref,
    update
}) => {

    // Signal to hold the current value of the slider.
    const current = useSignal(initial);

    /**
     * @brief Ensures the initial value obeys the minimum and maximum ranges.
     */
    useEffect(() => {
        if(current.value < min){
            current.value = min;
            update(structure_ref, current.value);
        }
        if(current.value > max){
            current.value = max;
            update(structure_ref, current.value);
        }
    }, [])

    /**
     * @brief Updates the slider and the external structure with a new value.
     * @param event The event holding the new value of the slider.
     */
    const valueChanged = async event => {
        current.value = event.target.value;
        update(structure_ref, current.value);
    }

    return (
        <div>
            <div>
                <span style={{fontSize: '1.2rem'}}>{label}</span>
                {tooltip && (
                    <TooltipWrapper
                      id={tooltip.id}
                      content={tooltip.content}
                      offset={tooltip.offset}
                      style={tooltip.style}
                      label={tooltip.label}
                    />
                )}
            </div>
            <div>
                <input
                    className={style.slider}
                    type="range"
                    id="slider"
                    name="slider"
                    min={min}
                    max={max}
                    value={initial}
                    onChange={valueChanged}
                />
                <input
                    className={style.spin_button}
                    type="number"
                    id="spinner"
                    name="spinner"
                    min={min}
                    max={max}
                    value={initial}
                    onChange={valueChanged}
                />
            </div>
        </div>
    );
}

export default NumericSlider;
