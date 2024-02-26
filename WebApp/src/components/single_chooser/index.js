import { useSignal } from "@preact/signals";
import { useEffect } from "preact/hooks";

const SimpleChooser = ({
    label,
    options,
    noSelection,
    initial,
    structure_ref,
    update
}) => {

    const current = useSignal(initial);

    // Check if the initial input is greater than allowed.
    useEffect(() => {
        var max = 0;
        options.map((option) => {
            if(option.idx > max){
                max = option.idx;
            }
        })
        if(max > initial){
            current.value = max;
            update(structure_ref, current.value);
        }
        if(initial < 0){
            current.value = 0;
            update(structure_ref, current.value);
        }
    }, [])

    const changeSelection = async (event) => {

        const selection = Number(event.target.value);

        if(current.value == selection){
            if(noSelection){
                current.value = 0;
            }
        }else{
            current.value = selection;
        }
        update(structure_ref, current.value);

    }

    const options_list = options.map((option) => {
        return <span>
            <input
                type="checkbox"
                id={option.option}
                name="list"
                value={option.idx}
                onChange={changeSelection}
                checked={current.value==option.idx}
            />
            <label for={option.option}>{option.option}</label>

        </span>
        
       
        
    });

    return (
        <div>
            {label}:
            <br></br>
            {options_list}
        </div>
                
    )
}

export default SimpleChooser;