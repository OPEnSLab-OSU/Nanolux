import { useSignal } from '@preact/signals';
import {h, Component} from 'preact';
import Select from 'preact-material-components/Select';

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
        ["Default"]
    ]

    const current = useSignal(initial);

    return (
        <div>
            <Select
                selectedIndex={current.value}
                onChange={(e)=>{
                    current.value = e.target.selectedIndex;
                    update(structureRef, current);
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
            </Select>
        </div>
    );
}

export default ConfigDropDown;