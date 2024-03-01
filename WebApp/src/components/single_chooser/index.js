
/**
 * @brief A simple single-choice checkbox selector
 * 
 * @param label The string label to show alongside the selector.
 * @param options The list of options and their IDs.
 * @param noSelection If unchecking a box is allowed. Produces an ID of 0.
 * @param initial The initial ID to select.
 * @param structure_ref The string reference to store values at.
 * @param update A function to update an external data structure.
 */
const SimpleChooser = ({
    label,
    options,
    noSelection,
    initial,
    structure_ref,
    update
}) => {

    /**
     * @brief A function to update an external structure with a new value.
     * @param event The calling event, which contains the ID of the clicked checkbox.
     */
    const changeSelection = async (event) => {

        const selection = Number(event.target.value);

        if(initial == selection){
            if(noSelection){
                update(structure_ref, 0);
            }
        }else{
            update(structure_ref, selection);
        }

    }

    /**
     * @brief An object that holds a list of HTML option inputs and labels.
     */
    const options_list = options.map((option) => {
        return <span>
            <input
                type="checkbox"
                id={option.option}
                name={option.option}
                value={option.idx}
                onChange={changeSelection}
                checked={initial===option.idx}
            />
            <label for={option.option}>{option.option}</label>
        </span>
    });

    /**
     * @brief Creates the chooser UI element.
     */
    return (
        <div>
            {label}:
            <br></br>
            {options_list}
        </div>
                
    )
}

export default SimpleChooser;