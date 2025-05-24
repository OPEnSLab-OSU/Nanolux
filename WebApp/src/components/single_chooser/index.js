import TooltipWrapper from '../tooltip/tooltip_wrapper';
import style from './style.css';

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
    tooltip,
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

        if(initial != selection){
            update(structure_ref, selection);
            event.target.checked = true;
        }else{
            if(noSelection){
                update(structure_ref, 0);
                event.target.checked = false;
            }else{
                event.target.checked = true;
            }
        }

    }

    /**
     * @brief An object that holds a list of HTML option inputs and labels.
     */
    const options_list = options.map((option) => {
        return <span>
            <label key={option.idx} className={style.option_group}>
                <input
                    type="checkbox"
                    id={option.option}
                    name={option.option}
                    value={option.idx}
                    onChange={changeSelection}
                    checked={initial===option.idx}
                />
                <TooltipWrapper
                  id={option.option}
                  content={option.tooltip}
                  label={option.option}
                />
            </label>
        </span>
    });

    /**
     * @brief Creates the chooser UI element.
     */
    return (
        <div>
            <div>
                {tooltip && (
                    <TooltipWrapper
                      id={tooltip.id}
                      content={tooltip.content}
                      offset={tooltip.offset}
                      style={tooltip.style}
                      label={label}
                    />
                )}
            </div>
            {options_list}
        </div>
                
    )
}

export default SimpleChooser;