import {useSignal} from "@preact/signals";
import style from './style.css';

const NumericSlider = ({
    label,
    min,
    max,
    initial,
    structure_ref,
    update
}) => {

    const current = useSignal(initial);

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
                    value={current}
                    onChange={valueChanged}
                />
                <input
                    className={style.spin_button}
                    type="number"
                    id="spinner"
                    name="spinner"
                    min={min}
                    max={max}
                    value={current}
                    onChange={valueChanged}
                />
            </div>
        </div>
    );
}

export default NumericSlider;
