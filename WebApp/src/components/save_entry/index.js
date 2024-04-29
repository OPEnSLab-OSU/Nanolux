import {loadSaveSlot, saveToSlot} from "../../utils/api";

/**
 * @brief A UI element that allows a user to save or load a pattern 
 * to/from a particular slot ID.
 * 
 * @param name The displayed name of the save slot.
 * @param idx The ID of the slot to save/load to/from.
 * 
 * @returns The UI element itself.
 */
const Save_Entry = ({
    name,
    idx
}) => {

    /**
     * @brief Forces the NanoLux device to save the currently loaded
     * pattern to the save slot given by the parameter "idx."
     */
    const save = async () => {
		var success = await saveToSlot(idx);
		if(!success){
			alert("Failed to save slot.");
		}
	}

    /**
     * @brief Forces the NanoLux device to load the pattern saved in
     * the save slot given by the parameter "idx."
     */
	const load = async () => {
		await loadSaveSlot(idx);
		window.location.reload(true);
	}

    return (
        <div >
            <tr>
                <th width='150'>{name}</th>
                <th>
                    <button onClick={load}>Load Pattern</button>
                </th>
                <th>
                    <button onClick={save}>Save Pattern</button>
                </th>
                
            </tr>
        </div>
    );
}

export default Save_Entry;
