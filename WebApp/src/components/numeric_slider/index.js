import { h } from 'preact';
import style from './style.css';
import {useSignal} from "@preact/signals";
import {useEffect, useState} from "preact/hooks";
import { useConnectivity } from '../../context/online_context';
import useInterval from '../../utils/use_interval';

const NumericSlider = ({
    label,
    min,
    max,
    getterFunction,
    saveFunction,
    api_key
}) => {
    const current = useSignal(min);
    const { isConnected } = useConnectivity();
    const [loading, setLoading] = useState(true);

    var enableRemoteUpdate = true;

    // Initalization
    useEffect(() => {
        if (isConnected) {
            getterFunction().then(data => current.value = data[api_key]);
            setLoading(false);
        }
        
    }, [isConnected])

    // Periodic
    useInterval(() => {
        if (isConnected && enableRemoteUpdate) {
            getterFunction().then(data => {current.value = data[api_key]});     
        }
    }, 1000);

    // Slider Updater
    const valueChanged = async event => {
        enableRemoteUpdate = false;
        current.value = event.target.value;
		if (isConnected) {
			await saveFunction(current.value);
		}
        enableRemoteUpdate = true;
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
