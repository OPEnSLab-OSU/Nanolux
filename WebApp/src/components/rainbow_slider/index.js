import React, {useState} from 'react';
import {useSignal} from "@preact/signals";
import style from './style.css';
import { useEffect } from "preact/hooks";

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
const RainbowSlider = ({
    label,
    min,
    max,
    initial,
    update
}) => {

    // Signal to hold the current value of the slider.
    const current = useSignal(initial);
    const [range1, setRange1] = useState(20);
    const [range2, setRange2] = useState(80);

    /**
     * @brief Ensures the initial value obeys the minimum and maximum ranges.
     */
    useEffect(() => {
        if(current.value < min){
            current.value = min;
            update(current.value);
        }
        if(current.value > max){
            current.value = max;
            update(current.value);
        }
    }, [])



    /**
     * @brief Updates the slider and the external structure with a new value.
     * @param event The event holding the new value of the slider.
     */
    
    const valueChanged = async event => {
        current.value = event.target.value;
    }

   /*
    const valueChanged = (e, thumb) => {
        const value = parseInt(e.target.value);
        if (thumb === 'left'){
            setRange1(value);
        } else{
            setRange2(value);
        }
    }
    */

    return (
        <div>
            <div>
                <span>{label}</span>
            </div>
            <div className={style.rainbow_input}>
                <input
                    //className={style.rainbow_input}
                    className={style.min}
                    type="range"
                    id="rainbow"
                    name="rainbow"
                    min={min}
                    max={max}
                    value={range1}
                    //onChange={valueChanged}
                    onChange={({ target }) => setRange1(Number(target.value))}
                />
                <input
                    //className={style.rainbow_input}
                    className={style.max}
                    type="range"
                    id="rainbow"
                    name="rainbow"
                    min={min}
                    max={max}
                    value={range2}
                    //onChange={valueChanged}
                    onChange={({ target }) => setRange2(Number(target.value))}
                />
            </div>
        </div>
    );
}

export default RainbowSlider;
