import { useSignal } from '@preact/signals';
import Select from 'preact-material-components/Select';
import { useEffect } from 'preact/hooks';
// import useInterval from '../../utils/use_interval';
import style from '../patterns/style.css';

// SOURCE:
// https://material.preactjs.com/component/select/
const ConfigDropDown = ({
    patternIdx,
    initial,
    structureRef,
    update
}) => {
    
    const configs = [
        ["None"],
        ["Default"],
        ["Default"],
        ["Default", "Blur", "Shift"],
        ["Default", "Octaves", "Shift", "Compression"],
        ["Default", "Shift"],
        ["Hue", "Formants", "Moving"],
        ["Default", "Sections", "Talk"],
        ["Basic", "Advanced", "Formant"],
        ["Default"],
        ["Default"],
        ["Frequency", "Volume"],
        ["Default"],
        ["Volume", "Frequency"],
        ["Default"]
    ]

    const current = useSignal(initial);

    useEffect(() => {
        if (current.value >= (configs[patternIdx]?.length ?? 0)) {
            current.value = 0;
            update(structureRef, 0);
        }
    }, [patternIdx]);

    const handleChange = e => {
        const idx = Number(e.target.value);
        current.value = idx;
        update(structureRef, idx);
    }

    return (
        <div>
            {/* <Select
                
                selectedIndex={current.value}
                onChange={(e)=>{
                    current.value = e.target.selectedIndex;
                    update(structureRef, current.value);
            }}>
                {(patternIdx <= configs.length && patternIdx >= 0) ?
                    
                    configs[patternIdx].map((optionName) => {
                        return <Select.Item>{optionName}</Select.Item>;
                    })
                    
                    :
                    configs[0].map((optionName) => {
                        return <Select.Item>{optionName}</Select.Item>;
                    })

                }
            </Select> */}
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