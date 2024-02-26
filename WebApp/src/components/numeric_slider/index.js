import {useSignal} from "@preact/signals";
import style from './style.css';
import { useEffect } from "preact/hooks";

const NumericSlider = ({
    label,
    min,
    max,
    initial,
    structure_ref,
    update
}) => {

    const current = useSignal(initial);

    // Check bounds and change them if data is out of bounds.
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

    // Slider Updater
    const valueChanged = async event => {
        current.value = event.target.value;
        update(structure_ref, current.value);
    }

    return (
        <div>
            <div>
                <span>{label}</span>
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
